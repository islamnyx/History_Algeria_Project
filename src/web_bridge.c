/*
 * web_bridge.c — Mongoose HTTP bridge for Algeria History DB v3.0
 * NSCS · Algorithms & Dynamic Data Structures · 2025–2026
 *
 * ROOT CAUSE FIXES vs previous version:
 *   1. Removed unconditional #include <windows.h>
 *      GTK already pulls in Win32 headers on Windows — including windows.h
 *      again causes type-redefinition compile errors. On Linux it was simply
 *      missing and caused a hard error. Sleep() is now wrapped in a portable
 *      mg_sleep_ms() helper using #ifdef _WIN32.
 *
 *   2. Removed extern Tlist + duplicate struct redefinition
 *      The old version declared its own "typedef struct Node {...} Tlist" and
 *      read g_personalities directly. Because main.c XOR-decrypts the DB into
 *      the list at runtime, the web thread was reading the pointer before
 *      decryption finished, getting garbage/empty data.
 *
 *   3. Now calls serialize_personalities_json() / serialize_events_json()
 *      — functions defined in main.c that walk the already-decrypted list.
 *
 * Compile (Windows):
 *   gcc main.c web_bridge.c mongoose.c \
 *       $(pkg-config --cflags --libs gtk+-3.0) -lm -lpthread -lws2_32 \
 *       -o algeria_history.exe
 *
 * Compile (Linux / Mac):
 *   gcc main.c web_bridge.c mongoose.c \
 *       $(pkg-config --cflags --libs gtk+-3.0) -lm -lpthread \
 *       -o algeria_history
 */

#include "mongoose.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Portable sleep ──────────────────────────────────────────────────────── */
#ifdef _WIN32
  /* GTK already included windows.h — Sleep() is already declared */
  static void mg_sleep_ms(unsigned ms) { Sleep(ms); }
#else
  #include <unistd.h>
  static void mg_sleep_ms(unsigned ms) { usleep((useconds_t)ms * 1000u); }
#endif

/* ══════════════════════════════════════════════════════════════════════════
   Functions defined in main.c — walk the already-decrypted linked lists
   and return malloc'd JSON strings. Caller must free().
   ══════════════════════════════════════════════════════════════════════════ */
extern char *serialize_personalities_json(void);
extern char *serialize_events_json(void);
extern void  save_database(void);

/* ══════════════════════════════════════════════════════════════════════════
   Minimal node mirror — used ONLY for /api/add and /api/delete.
   Layout matches main.c exactly: name[120], definition[400], dob, dod, *next
   ══════════════════════════════════════════════════════════════════════════ */
typedef struct { int day, month, year; } wb_date;
typedef struct WBNode {
    char    name[120];
    char    definition[400];
    wb_date dob, dod;
    struct WBNode *next;
} WBNode;

extern WBNode *g_personalities;  /* same memory as main.c's g_personalities */

/* ── Common CORS headers ─────────────────────────────────────────────────── */
#define JSON_CORS \
    "Content-Type: application/json\r\n"  \
    "Access-Control-Allow-Origin: *\r\n"  \
    "Cache-Control: no-cache\r\n"

/* ══════════════════════════════════════════════════════════════════════════
   GET /api/personalities
   ══════════════════════════════════════════════════════════════════════════ */
static void handle_get_personalities(struct mg_connection *c) {
    char *json = serialize_personalities_json();
    if (!json) { mg_http_reply(c, 500, JSON_CORS, "serialization error\n"); return; }
    mg_http_reply(c, 200, JSON_CORS, "%s", json);
    free(json);
}

/* ══════════════════════════════════════════════════════════════════════════
   GET /api/events
   ══════════════════════════════════════════════════════════════════════════ */
static void handle_get_events(struct mg_connection *c) {
    char *json = serialize_events_json();
    if (!json) { mg_http_reply(c, 500, JSON_CORS, "serialization error\n"); return; }
    mg_http_reply(c, 200, JSON_CORS, "%s", json);
    free(json);
}

/* ══════════════════════════════════════════════════════════════════════════
   GET /api/status
   ══════════════════════════════════════════════════════════════════════════ */
static void handle_status(struct mg_connection *c) {
    char *pj = serialize_personalities_json();
    char *ej = serialize_events_json();
    int pc = 0, ec = 0;
    if (pj) { for (const char *p = pj; *p; p++) if (*p == '{') pc++; free(pj); }
    if (ej) { for (const char *p = ej; *p; p++) if (*p == '{') ec++; free(ej); }
    mg_http_reply(c, 200, JSON_CORS,
        "{\"status\":\"ok\",\"personalities\":%d,\"events\":%d}", pc, ec);
}

/* ══════════════════════════════════════════════════════════════════════════
   POST /api/add
   Body: {"name":"...","def":"...","dob":YEAR,"dod":YEAR}
   ══════════════════════════════════════════════════════════════════════════ */
static void handle_add(struct mg_connection *c, struct mg_http_message *hm) {
    char name[120] = {0}, def[400] = {0};

    char *s_name = mg_json_get_str(hm->body, "$.name");
    char *s_def  = mg_json_get_str(hm->body, "$.def");
    long  dob    = mg_json_get_long(hm->body, "$.dob", 0);
    long  dod_y  = mg_json_get_long(hm->body, "$.dod", 0);

    if (s_name) { strncpy(name, s_name, 119); free(s_name); }
    if (s_def)  { strncpy(def,  s_def,  399); free(s_def);  }

    if (!name[0]) {
        mg_http_reply(c, 400, JSON_CORS, "{\"ok\":false,\"error\":\"name required\"}");
        return;
    }

    WBNode *node = (WBNode *)calloc(1, sizeof(WBNode));
    if (!node) {
        mg_http_reply(c, 500, JSON_CORS, "{\"ok\":false,\"error\":\"out of memory\"}");
        return;
    }
    strncpy(node->name,       name, 119);
    strncpy(node->definition, def,  399);
    node->dob.year = (int)dob;   node->dob.day = 1; node->dob.month = 1;
    node->dod.year = (int)dod_y; node->dod.day = 1; node->dod.month = 1;
    node->next = NULL;

    if (!g_personalities) {
        g_personalities = node;
    } else {
        WBNode *tail = g_personalities;
        while (tail->next) tail = tail->next;
        tail->next = node;
    }
    save_database();

    mg_http_reply(c, 200, JSON_CORS, "{\"ok\":true}");
}

/* ══════════════════════════════════════════════════════════════════════════
   GET /api/delete?name=X
   ══════════════════════════════════════════════════════════════════════════ */
static void handle_delete(struct mg_connection *c, struct mg_http_message *hm) {
    char name[120] = {0};
    mg_http_get_var(&hm->query, "name", name, sizeof(name));

    if (!name[0]) {
        mg_http_reply(c, 400, JSON_CORS, "{\"ok\":false,\"error\":\"name param required\"}");
        return;
    }

    WBNode *prev = NULL, *cur = g_personalities;
    int found = 0;
    while (cur) {
        if (strcasecmp(cur->name, name) == 0) {
            if (prev) prev->next = cur->next;
            else      g_personalities = cur->next;
            free(cur); found = 1; break;
        }
        prev = cur; cur = cur->next;
    }
    if (found) save_database();

    mg_http_reply(c, 200, JSON_CORS, "{\"ok\":%s}", found ? "true" : "false");
}

/* ══════════════════════════════════════════════════════════════════════════
   EVENT HANDLER
   ══════════════════════════════════════════════════════════════════════════ */
static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    if (mg_match(hm->method, mg_str("OPTIONS"), NULL)) {
        mg_http_reply(c, 204,
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n", "");
        return;
    }

    if      (mg_match(hm->uri, mg_str("/api/personalities"), NULL)) handle_get_personalities(c);
    else if (mg_match(hm->uri, mg_str("/api/events"),        NULL)) handle_get_events(c);
    else if (mg_match(hm->uri, mg_str("/api/status"),        NULL)) handle_status(c);
    else if (mg_match(hm->uri, mg_str("/api/add"),           NULL)) handle_add(c, hm);
    else if (mg_match(hm->uri, mg_str("/api/delete"),        NULL)) handle_delete(c, hm);
    else {
        struct mg_http_serve_opts opts = {.root_dir = "."};
        mg_http_serve_dir(c, hm, &opts);
    }
}

/* ══════════════════════════════════════════════════════════════════════════
   BACKGROUND THREAD
   Waits 2 s so GTK + load_database() finish before we start serving.
   ══════════════════════════════════════════════════════════════════════════ */
static void *mongoose_thread(void *arg) {
    (void)arg;
    mg_sleep_ms(2000);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    struct mg_connection *conn = mg_http_listen(
        &mgr, "http://0.0.0.0:8080", event_handler, NULL);

    if (!conn) {
        fprintf(stderr,
            "[web_bridge] ERROR: cannot bind port 8080\n"
            "[web_bridge] Try changing 8080 to 8181 in web_bridge.c\n");
        mg_mgr_free(&mgr);
        return NULL;
    }

    fprintf(stderr,
        "\n[web_bridge] HTTP server ready\n"
        "[web_bridge]   API: http://localhost:8080/api/personalities\n"
        "[web_bridge]   GUI: http://localhost:8080/algeria_history_v2.html\n\n");

    for (;;) mg_mgr_poll(&mgr, 100);

    mg_mgr_free(&mgr);
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════════════
   PUBLIC — call once from main() after load_database()
   ══════════════════════════════════════════════════════════════════════════ */
void web_bridge_start(void) {
    pthread_t tid;
    if (pthread_create(&tid, NULL, mongoose_thread, NULL) != 0) {
        fprintf(stderr, "[web_bridge] ERROR: could not create HTTP thread\n");
        return;
    }
    pthread_detach(tid);
}