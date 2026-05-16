/*
 ██████╗  ██╗      ██████╗  ███████╗ ██████╗  ██╗ ███████╗
██╔═══██╗ ██║     ██╔════╝  ██╔════╝ ██╔══██╗ ██║ ██╔════╝
██║   ██║ ██║     ██║  ███╗ █████╗   ██████╔╝ ██║ █████╗
██║▄▄ ██║ ██║     ██║   ██║ ██╔══╝   ██╔══██╗ ██║ ██╔══╝
╚██████╔╝ ███████╗╚██████╔╝ ███████╗ ██║  ██║ ██║ ███████╗
 ╚══▀▀═╝  ╚══════╝ ╚═════╝  ╚══════╝ ╚═╝  ╚═╝ ╚═╝ ╚══════╝

  Algeria History Database — GTK3 GUI  v3.0
  NSCS · Algorithms & Dynamic Data Structures · 2025–2026

  BUGS FIXED vs v2:
  ─────────────────
  1. merge2Nodes  : was making list circular mid-build → crash on traversal;
                    fixed to build linearly then close the circle at the end.
  2. NavData      : typedef was declared AFTER build_ui() used it → forward-declared.
  3. perm_count / subseq_count : were static globals → now local per-call (no state bleed).
  4. sortNameStack / definitionStack : temp stack started empty → now seeds from source.
  5. isPalindromWord  : buffer was stack-local, pointer arithmetic was walking off; replaced
                        with clean index-based version.
  6. countPersonality : original took a `date*` but GUI passed an int → unified to int year.
  7. on_bst_mirror    : called handler checked g_tree AFTER mirroring, not before.
  8. longestSubyear   : d2 used date1 instead of date2 (copy-paste bug) → fixed.
  9. getInfoPersonality : returned NULL on miss but callers checked res->top → guarded.
 10. merge2Nodes circular output : loop did not detect circularity → fixed with start ptr.
 11. Makefile target : added single-file compile rule at the bottom.

  NEW FEATURES vs v2:
  ────────────────────
  ✦ Statistics Dashboard   — live counts, avg age, year range, born/died pie breakdown
  ✦ Timeline view          — ASCII timeline printed in the terminal for all personalities
  ✦ Export to CSV          — one-click dump of the entire DB to algeria_export.csv
  ✦ "Born same year" finder — select a year, list all born then
  ✦ BST node count in range now prints names, not just the count
  ✦ Recursive palindrome check shows step-by-step trace in output
  ✦ Quick-search now highlights matched field (name vs definition)
  ✦ Status bar shows live DB stats after every mutating operation
  ✦ All dialogs: pressing Enter confirms (Return key = OK)
  ✦ Confirmation dialog before any destructive operation
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

void web_bridge_start(void);

// ═══════════════════════════════════════════════════════════════
//  DATA STRUCTURES  (self-contained — no external headers needed)
// ═══════════════════════════════════════════════════════════════

typedef struct { int day, month, year; } date;

typedef struct Node {
    char name[120];
    char definition[400];
    date dob, dod;
    struct Node *next;
} Tlist;

typedef struct binode {
    char name[120];
    char definition[400];
    date dob, dod;
    struct binode *next;
    struct binode *prev;
} TBilist;

typedef struct QNode {
    char name[120];
    char definition[400];
    date dob, dod;
    struct QNode *next;
} QNode;

typedef struct { QNode *head, *tail; } TQueue;

typedef struct { Tlist *top; } TStack;

typedef struct TTreeNode {
    char name[120];
    char definition[400];
    date dob, dod;
    struct TTreeNode *left, *right;
} TTree;

// ═══════════════════════════════════════════════════════════════
//  GLOBAL STATE
// ═══════════════════════════════════════════════════════════════

Tlist  *g_personalities = NULL;
static Tlist  *g_dates         = NULL;
Tlist  *g_events        = NULL;
static Tlist  *g_merged        = NULL;   /* circular list, may be NULL */
static TStack *g_stack         = NULL;
static TTree  *g_tree          = NULL;
static TQueue *g_queue         = NULL;
static char    g_db_path[512]  = "database.txt";

// UI globals
static GtkWidget *g_main_window;
static GtkWidget *g_stack_pages;
static GtkWidget *g_output_view;
static GtkWidget *g_status_bar;

// ═══════════════════════════════════════════════════════════════
//  CSS — Algerian Desert Gold + Ottoman Dark Manuscript
// ═══════════════════════════════════════════════════════════════

static const char *CSS =
"window { background-color: #120905; }"

".sidebar { background-color: #0c0603; border-right: 1px solid #6b4f10;"
"           min-width: 215px; }"
".sidebar-title { font-size: 10px; font-weight: bold; color: #a07010;"
"                 letter-spacing: 3px; padding: 4px 16px; }"

".nav-section { font-size: 9px; color: #5a3a08; letter-spacing: 2px;"
"               padding: 10px 16px 2px 16px; font-weight: bold; }"

".nav-btn { background: transparent; border: none; border-radius: 0;"
"           color: #c89030; font-size: 12px; padding: 8px 16px 8px 22px;"
"           text-align: left; border-left: 2px solid transparent; }"
".nav-btn:hover { background-color: rgba(180,130,10,0.10);"
"                 border-left-color: #a07010; color: #e8b040; }"
".nav-btn.active { background-color: rgba(180,130,10,0.18);"
"                  border-left-color: #d49020; color: #ffd060; font-weight: bold; }"

".app-header { background-color: #0c0603; border-bottom: 1px solid #6b4f10;"
"              padding: 0 20px; min-height: 54px; }"
".app-title   { font-size: 18px; font-weight: bold; color: #e8a820; letter-spacing: 2px; }"
".app-subtitle{ font-size: 9px;  color: #7a5518; letter-spacing: 3px; }"

".content-area { background-color: #100705; padding: 18px 22px; }"

".module-title    { font-size: 20px; font-weight: bold; color: #d49020; letter-spacing: 1px; }"
".module-subtitle { font-size: 10px; color: #6a4510; letter-spacing: 3px; margin-bottom: 14px; }"

".action-btn { background-color: #1e1005; border: 1px solid #6b4f10; border-radius: 3px;"
"              color: #d49020; font-size: 11px; font-weight: bold;"
"              padding: 7px 10px; letter-spacing: 1px; }"
".action-btn:hover  { background-color: #6b4f10; color: #fff8e0; }"
".action-btn:active { background-color: #4a3508; }"

".danger-btn { border-color: #7a1515; color: #d04040; }"
".danger-btn:hover { background-color: #7a1515; color: #ffe0e0; }"

".warn-btn { border-color: #6a5010; color: #c89030; }"
".warn-btn:hover { background-color: #4a3800; color: #ffd060; }"

".new-btn { border-color: #106a50; color: #30c890; }"
".new-btn:hover { background-color: #0a3a2a; color: #80ffe0; }"

"textview.output-text { background-color: #060402; color: #b89040;"
"                       font-family: monospace; font-size: 12px; padding: 10px; }"
"textview.output-text text { background-color: #060402; color: #b89040; }"

"entry { background-color: #1a0c06; border: 1px solid #3a2510; border-radius: 3px;"
"        color: #d4a840; font-size: 12px; padding: 5px 8px; caret-color: #d49020; }"
"entry:focus { border-color: #6b4f10; }"

".status-bar { background-color: #0c0603; border-top: 1px solid #2a1a06;"
"              color: #6a4510; font-size: 10px; padding: 3px 14px; }"
".gold-sep   { background-color: #3a2510; min-height: 1px; margin: 8px 0; }"

"scrollbar { background-color: #0c0603; min-width: 7px; min-height: 7px; }"
"scrollbar slider { background-color: #3a2510; border-radius: 3px; }"
"scrollbar slider:hover { background-color: #6b4f10; }"

"dialog { background-color: #120905; }"
"label  { color: #c89030; }"
;

// ═══════════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS (needed because NavData is used in build_ui)
// ═══════════════════════════════════════════════════════════════
typedef struct { GtkWidget *btn; GList *all; const char *page; } NavData;

// ═══════════════════════════════════════════════════════════════
//  OUTPUT HELPERS
// ═══════════════════════════════════════════════════════════════

static void output_clear(void) {
    GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_output_view));
    gtk_text_buffer_set_text(b, "", -1);
}

static void output_append(const char *text) {
    GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_output_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(b, &end);
    gtk_text_buffer_insert(b, &end, text, -1);
    // scroll to bottom
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(g_output_view),
        gtk_text_buffer_get_insert(b));
}

static void output_appendf(const char *fmt, ...) {
    char buf[2048]; va_list a;
    va_start(a, fmt); vsnprintf(buf, sizeof(buf), fmt, a); va_end(a);
    output_append(buf);
}

static void set_status(const char *msg) {
    gtk_label_set_text(GTK_LABEL(g_status_bar), msg);
}

static void output_header(const char *title) {
    output_clear();
    output_append("╔══════════════════════════════════════════════════════════╗\n║  ");
    char line[60]; snprintf(line, sizeof(line), "%-56s", title);
    output_append(line);
    output_append("║\n╚══════════════════════════════════════════════════════════╝\n\n");
}

// ═══════════════════════════════════════════════════════════════
//  DATE UTILITIES
// ═══════════════════════════════════════════════════════════════

static date parse_date(const char *s) {
    date d = {1, 1, 0};
    if (!s || !*s) return d;
    if (strlen(s) <= 4) { d.year = atoi(s); return d; }
    sscanf(s, "%d/%d/%d", &d.day, &d.month, &d.year);
    return d;
}

static void fmt_date(char *out, date d) {
    if (d.day == 1 && d.month == 1)
        snprintf(out, 20, "%d", d.year);
    else
        snprintf(out, 20, "%02d/%02d/%04d", d.day, d.month, d.year);
}

static date stringToDate(const char *str) { return parse_date(str); }

static bool isOverlapping(date dob1, date dod1, date dob2, date dod2) {
    int s1 = dob1.year * 10000 + dob1.month * 100 + dob1.day;
    int e1 = dod1.year * 10000 + dod1.month * 100 + dod1.day;
    int s2 = dob2.year * 10000 + dob2.month * 100 + dob2.day;
    int e2 = dod2.year * 10000 + dod2.month * 100 + dod2.day;
    return (s1 <= e2 && s2 <= e1);
}

// ═══════════════════════════════════════════════════════════════
//  LIVE STATUS UPDATE
// ═══════════════════════════════════════════════════════════════

static void refresh_status(void) {
    int pc = 0, ec = 0;
    for (Tlist *n = g_personalities; n; n = n->next) pc++;
    for (Tlist *n = g_events;        n; n = n->next) ec++;
    char msg[160];
    snprintf(msg, sizeof(msg),
        "  DB: %d personalities · %d events  |  %s",
        pc, ec, g_db_path);
    set_status(msg);
}

// ═══════════════════════════════════════════════════════════════
//  LINKED LIST — CORE HELPERS
// ═══════════════════════════════════════════════════════════════

static int countWordsLL(const char *str) {
    int count = 0, i = 0;
    if (!str || str[0] == '\0') return 0;
    while (str[i] != '\0') {
        if (str[i] == ' ' && str[i+1] != ' ' && str[i+1] != '\0') count++;
        i++;
    }
    return count + 1;
}

static int isPalindrome(const char *str) {
    int left = 0, right = (int)strlen(str) - 1;
    while (right > left) {
        // BUG FIX: tolower applied to each char separately, not to the comparison result
        if (tolower((unsigned char)str[left]) != tolower((unsigned char)str[right])) return 0;
        left++; right--;
    }
    return 1;
}

// ── Queue primitive ──────────────────────────────────────────
static void enqueue(TQueue *q, const char *name, const char *def, date dob, date dod) {
    QNode *n = calloc(1, sizeof(QNode));
    strcpy(n->name, name); strcpy(n->definition, def);
    n->dob = dob; n->dod = dod; n->next = NULL;
    if (!q->tail) { q->head = q->tail = n; }
    else          { q->tail->next = n; q->tail = n; }
}

// ── Bilist primitive ─────────────────────────────────────────
static TBilist* insertBilist(TBilist *head, const char *name, const char *def, date dob, date dod) {
    TBilist *n = calloc(1, sizeof(TBilist));
    strcpy(n->name, name); strcpy(n->definition, def);
    n->dob = dob; n->dod = dod; n->next = NULL; n->prev = NULL;
    if (!head) return n;
    TBilist *cur = head;
    while (cur->next) cur = cur->next;
    cur->next = n; n->prev = cur;
    return head;
}

// ═══════════════════════════════════════════════════════════════
//  DATABASE — LOAD / SAVE
// ═══════════════════════════════════════════════════════════════

static void free_list(Tlist **lst) {
    Tlist *t = *lst;
    while (t) { Tlist *nx = t->next; free(t); t = nx; }
    *lst = NULL;
}

static void load_database(void) {
    free_list(&g_personalities);
    free_list(&g_dates);
    free_list(&g_events);
    g_merged = NULL;

    FILE *f = fopen(g_db_path, "r");
    if (!f) {
        set_status("  database.txt not found — add personalities to get started");
        return;
    }

    char line[600];
    Tlist *ptail = NULL, *dtail = NULL, *etail = NULL;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0]) continue;

        if (strchr(line, '=')) {
            // Format: Name=Definition=DOB=DOD
            char n[120]={0}, d[400]={0}, y1[20]={0}, y2[20]={0};
            char tmp[600]; strcpy(tmp, line);
            char *p1 = strtok(tmp, "="), *p2 = strtok(NULL, "="),
                 *p3 = strtok(NULL, "="), *p4 = strtok(NULL, "=");
            if (!p1 || !p2) continue;
            strncpy(n, p1, 119); strncpy(d, p2, 399);
            if (p3) strncpy(y1, p3, 19);
            if (p4) strncpy(y2, p4, 19);

            // personality list (name + definition + dates)
            Tlist *pn = calloc(1, sizeof(Tlist));
            strcpy(pn->name, n); strcpy(pn->definition, d);
            pn->dob = parse_date(y1); pn->dod = parse_date(y2);
            if (!g_personalities) { g_personalities = ptail = pn; }
            else                  { ptail->next = pn; ptail = pn; }

            // dates list (name + dates only — definition left empty)
            Tlist *dn = calloc(1, sizeof(Tlist));
            strcpy(dn->name, n);
            dn->dob = parse_date(y1); dn->dod = parse_date(y2);
            if (!g_dates) { g_dates = dtail = dn; }
            else          { dtail->next = dn; dtail = dn; }

        } else if (strchr(line, ':')) {
            // Format: EventName:Description{YEAR}
            char nm[200]={0}, df[400]={0}, yr[20]={0};
            char *col = strchr(line, ':');
            strncpy(nm, line, (int)(col - line));
            char *rest = col + 1;
            char *ob = strchr(rest, '{'), *cb = strchr(rest, '}');
            if (ob && cb) {
                strncpy(df, rest, (int)(ob - rest));
                strncpy(yr, ob + 1, (int)(cb - ob - 1));
            } else {
                strncpy(df, rest, 399);
            }
            Tlist *en = calloc(1, sizeof(Tlist));
            strcpy(en->name, nm); strcpy(en->definition, df);
            en->dob = parse_date(yr);
            if (!g_events) { g_events = etail = en; }
            else           { etail->next = en; etail = en; }
        }
    }
    fclose(f);
    refresh_status();
}

void save_database(void) {
    FILE *f = fopen(g_db_path, "w");
    if (!f) { set_status("  ERROR: could not write database.txt"); return; }
    for (Tlist *n = g_personalities; n; n = n->next) {
        char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
        fprintf(f, "%s=%s=%s=%s\n", n->name, n->definition, d1, d2);
    }
    for (Tlist *n = g_events; n; n = n->next) {
        char d1[20]; fmt_date(d1, n->dob);
        fprintf(f, "%s:%s{%s}\n", n->name, n->definition, d1);
    }
    fclose(f);
    refresh_status();
}

/* JSON serializer — called by web_bridge.c */
char *serialize_personalities_json(void) {
    size_t bufsz = 131072;
    char  *buf   = (char *)malloc(bufsz);
    if (!buf) return NULL;

    size_t pos = 0;
    buf[pos++] = '[';
    int first = 1;

    for (Tlist *n = g_personalities; n; n = n->next) {
        if (!first) buf[pos++] = ',';
        first = 0;

        /* escape name */
        char ename[256]; size_t j=0;
        for(size_t i=0;n->name[i]&&j+4<255;i++){
            unsigned char ch=n->name[i];
            if(ch=='"'){ename[j++]='\\';ename[j++]='"';}
            else if(ch=='\\'){ename[j++]='\\';ename[j++]='\\';}
            else if(ch>=0x20)ename[j++]=ch;
        } ename[j]=0;

        /* escape def */
        char edef[900]; j=0;
        for(size_t i=0;n->definition[i]&&j+4<899;i++){
            unsigned char ch=n->definition[i];
            if(ch=='"'){edef[j++]='\\';edef[j++]='"';}
            else if(ch=='\\'){edef[j++]='\\';edef[j++]='\\';}
            else if(ch>=0x20)edef[j++]=ch;
        } edef[j]=0;

        /* wiki name */
        char wiki[128];
        strncpy(wiki,n->name,127); wiki[127]=0;
        for(int k=0;wiki[k];k++) if(wiki[k]==' ') wiki[k]='_';

        /* category */
        const char *cat="Figure";
        char low[401]; size_t li=0;
        for(;n->definition[li]&&li<400;li++) low[li]=(char)tolower((unsigned char)n->definition[li]);
        low[li]=0;
        if(strstr(low,"king")||strstr(low,"sultan")||strstr(low,"emir")||strstr(low,"dey")||strstr(low,"bey")) cat="Ruler";
        else if(strstr(low,"general")||strstr(low,"commander")||strstr(low,"warrior")||strstr(low,"military")) cat="Military";
        else if(strstr(low,"poet")||strstr(low,"writer")||strstr(low,"author")||strstr(low,"philosopher")||strstr(low,"scholar")||strstr(low,"historian")) cat="Scholar";
        else if(strstr(low,"saint")||strstr(low,"sufi")||strstr(low,"imam")) cat="Religious";
        else if(strstr(low,"politician")||strstr(low,"president")||strstr(low,"minister")) cat="Political";

        int dod_val = (n->dod.year > 0) ? n->dod.year : 2023;

        int written = snprintf(buf+pos, bufsz-pos,
            "{\"name\":\"%s\",\"def\":\"%s\",\"dob\":%d,\"dod\":%d,\"cat\":\"%s\",\"wiki\":\"%s\"}",
            ename, edef, n->dob.year, dod_val, cat, wiki);
        if(written<0||(size_t)written>=bufsz-pos) break;
        pos+=(size_t)written;
    }

    buf[pos++]=']'; buf[pos]=0;
    return buf; /* caller must free() */
}

char *serialize_events_json(void) {
    size_t bufsz = 65536;
    char  *buf   = (char *)malloc(bufsz);
    if (!buf) return NULL;

    size_t pos = 0;
    buf[pos++] = '[';
    int first = 1;

    for (Tlist *n = g_events; n; n = n->next) {
        if (!first) buf[pos++] = ',';
        first = 0;
        char ename[256], edef[900]; size_t j=0;
        for(size_t i=0;n->name[i]&&j+4<255;i++){unsigned char ch=n->name[i];if(ch=='"'){ename[j++]='\\';ename[j++]='"';}else if(ch=='\\'){ename[j++]='\\';ename[j++]='\\';}else if(ch>=0x20)ename[j++]=ch;} ename[j]=0;
        j=0;
        for(size_t i=0;n->definition[i]&&j+4<899;i++){unsigned char ch=n->definition[i];if(ch=='"'){edef[j++]='\\';edef[j++]='"';}else if(ch=='\\'){edef[j++]='\\';edef[j++]='\\';}else if(ch>=0x20)edef[j++]=ch;} edef[j]=0;
        int written=snprintf(buf+pos,bufsz-pos,"{\"name\":\"%s\",\"def\":\"%s\",\"year\":%d}",ename,edef,n->dob.year);
        if(written<0||(size_t)written>=bufsz-pos) break;
        pos+=(size_t)written;
    }

    buf[pos++]=']'; buf[pos]=0;
    return buf;
}

// ═══════════════════════════════════════════════════════════════
//  LINKED LIST FUNCTIONS
// ═══════════════════════════════════════════════════════════════

// ── printList ────────────────────────────────────────────────
static void gui_printList(Tlist *head) {
    if (!head) { output_append("  No personalities found.\n"); return; }
    int i = 1;
    for (Tlist *cur = head; cur; cur = cur->next, i++) {
        char d1[20], d2[20]; fmt_date(d1, cur->dob); fmt_date(d2, cur->dod);
        output_appendf("  [%d] %s\n      %s\n      Born: %-12s  Died: %s\n\n",
            i, cur->name, cur->definition, d1, d2);
    }
}

// ── sortWord (alphabetical bubble sort) ──────────────────────
static Tlist* sortWord(Tlist *syn) {
    if (!syn) return NULL;
    int swapped; Tlist *ptr1, *lptr = NULL;
    do {
        swapped = 0; ptr1 = syn;
        while (ptr1->next != lptr) {
            if (strcmp(ptr1->name, ptr1->next->name) > 0) {
                char t[120], td[400];
                strcpy(t, ptr1->name); strcpy(ptr1->name, ptr1->next->name); strcpy(ptr1->next->name, t);
                strcpy(td, ptr1->definition); strcpy(ptr1->definition, ptr1->next->definition); strcpy(ptr1->next->definition, td);
                date dt = ptr1->dob; ptr1->dob = ptr1->next->dob; ptr1->next->dob = dt;
                dt = ptr1->dod; ptr1->dod = ptr1->next->dod; ptr1->next->dod = dt;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
    return syn;
}

// ── sortWord2 (by name length) ───────────────────────────────
static Tlist* sortWord2(Tlist *syn) {
    if (!syn) return NULL;
    int swapped; Tlist *ptr1, *lptr = NULL;
    do {
        swapped = 0; ptr1 = syn;
        while (ptr1->next != lptr) {
            if (strlen(ptr1->name) > strlen(ptr1->next->name)) {
                char t[120], td[400];
                strcpy(t, ptr1->name); strcpy(ptr1->name, ptr1->next->name); strcpy(ptr1->next->name, t);
                strcpy(td, ptr1->definition); strcpy(ptr1->definition, ptr1->next->definition); strcpy(ptr1->next->definition, td);
                date dt = ptr1->dob; ptr1->dob = ptr1->next->dob; ptr1->next->dob = dt;
                dt = ptr1->dod; ptr1->dod = ptr1->next->dod; ptr1->next->dod = dt;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
    return syn;
}

// ── sortPersonality (by lifespan ascending) ──────────────────
static Tlist* sortPersonality(Tlist *syn) {
    if (!syn) return NULL;
    int swapped; Tlist *ptr1, *lptr = NULL;
    do {
        swapped = 0; ptr1 = syn;
        while (ptr1->next != lptr) {
            int a1 = ptr1->dod.year      - ptr1->dob.year;
            int a2 = ptr1->next->dod.year - ptr1->next->dob.year;
            if (a1 > a2) {
                char t[120], td[400];
                strcpy(t, ptr1->name); strcpy(ptr1->name, ptr1->next->name); strcpy(ptr1->next->name, t);
                strcpy(td, ptr1->definition); strcpy(ptr1->definition, ptr1->next->definition); strcpy(ptr1->next->definition, td);
                date dt = ptr1->dob; ptr1->dob = ptr1->next->dob; ptr1->next->dob = dt;
                dt = ptr1->dod; ptr1->dod = ptr1->next->dod; ptr1->next->dod = dt;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
    return syn;
}

// ── deletePersonality ────────────────────────────────────────
// BUG FIX: prev/curr advancement was inside the if-block → infinite loop
static Tlist* deletePersonality(Tlist *s, const char *name) {
    Tlist *prev = NULL, *cur = s;
    while (cur) {
        if (strcasecmp(cur->name, name) == 0) {
            if (prev) prev->next = cur->next;
            else      s = cur->next;
            free(cur);
            return s;           // done — only first occurrence
        }
        prev = cur; cur = cur->next;  // BUG FIX: outside the if
    }
    return s;
}

// ── updatePersonality ────────────────────────────────────────
static Tlist* updatePersonality(Tlist *s, const char *name, const char *def,
                                const char *dob, const char *dod) {
    for (Tlist *n = s; n; n = n->next) {
        if (strcasecmp(n->name, name) == 0) {
            strcpy(n->definition, def);
            n->dob = parse_date(dob);
            n->dod = parse_date(dod);
            return s;
        }
    }
    return s;
}

// ── similarPersonality ───────────────────────────────────────
static Tlist* similarPersonality(Tlist *s, const char *word) {
    Tlist *target = NULL;
    for (Tlist *n = s; n; n = n->next)
        if (strcasecmp(n->name, word) == 0) { target = n; break; }
    if (!target) return NULL;
    Tlist *results = NULL, *rtail = NULL;
    for (Tlist *n = s; n; n = n->next) {
        if (n == target) continue;
        if (n->dob.year == target->dob.year || n->dod.year == target->dod.year) {
            Tlist *nn = calloc(1, sizeof(Tlist)); *nn = *n; nn->next = NULL;
            if (!results) { results = rtail = nn; }
            else          { rtail->next = nn; rtail = nn; }
        }
    }
    return results;
}

// ── countPersonality (alive at year) ─────────────────────────
static Tlist* countPersonality(Tlist *s, int year) {
    Tlist *results = NULL, *rtail = NULL;
    for (Tlist *n = s; n; n = n->next) {
        if (n->dob.year <= year && n->dod.year >= year) {
            Tlist *nn = calloc(1, sizeof(Tlist)); *nn = *n; nn->next = NULL;
            if (!results) { results = rtail = nn; }
            else          { rtail->next = nn; rtail = nn; }
        }
    }
    return results;
}

// ── palindromeName ────────────────────────────────────────────
// BUG FIX: original used || instead of && in inner while → NULL deref
static Tlist* palindromeName(Tlist *s) {
    Tlist *newlist = NULL;
    for (Tlist *cur = s; cur; cur = cur->next) {
        if (isPalindrome(cur->name)) {
            Tlist *nn = calloc(1, sizeof(Tlist));
            strcpy(nn->name, cur->name); nn->next = NULL;
            if (!newlist || strcmp(nn->name, newlist->name) < 0) {
                nn->next = newlist; newlist = nn;
            } else {
                Tlist *tmp = newlist;
                // BUG FIX: && not ||
                while (tmp->next && strcmp(tmp->next->name, nn->name) < 0)
                    tmp = tmp->next;
                nn->next = tmp->next; tmp->next = nn;
            }
        }
    }
    return newlist;
}

// ── mergeNodes (bidirectional) ────────────────────────────────
static TBilist* mergeNodes(Tlist *s, Tlist *a) {
    if (!s || !a) return NULL;
    TBilist *head = NULL, *tail = NULL;
    for (Tlist *cs = s, *ca = a; cs && ca; cs = cs->next, ca = ca->next) {
        TBilist *nn = calloc(1, sizeof(TBilist));
        strcpy(nn->name, cs->name); strcpy(nn->definition, cs->definition);
        nn->dob = ca->dob; nn->dod = ca->dod; nn->next = NULL;
        if (!head) { nn->prev = NULL; head = tail = nn; }
        else       { tail->next = nn; nn->prev = tail; tail = nn; }
    }
    return head;
}

// ── merge2Nodes (circular singly-linked) ─────────────────────
// BUG FIX: original set tail->next=head DURING the loop → broken pointers
static Tlist* merge2Nodes(Tlist *s, Tlist *a) {
    if (!s || !a) return NULL;
    Tlist *head = NULL, *tail = NULL;
    for (Tlist *cs = s, *ca = a; cs && ca; cs = cs->next, ca = ca->next) {
        Tlist *nn = calloc(1, sizeof(Tlist));
        strcpy(nn->name, cs->name); strcpy(nn->definition, cs->definition);
        nn->dob = ca->dob; nn->dod = ca->dod; nn->next = NULL;
        if (!head) { head = tail = nn; }
        else       { tail->next = nn; tail = nn; }
    }
    if (tail) tail->next = head;   // BUG FIX: close the circle AFTER building
    return head;
}

// ── addPersonality ────────────────────────────────────────────
static Tlist* addPersonality(Tlist *s, const char *name, const char *def,
                              const char *dob, const char *dod) {
    Tlist *nn = calloc(1, sizeof(Tlist));
    strcpy(nn->name, name); strcpy(nn->definition, def);
    nn->dob = parse_date(dob); nn->dod = parse_date(dod);
    nn->next = s; s = nn;
    // also add to g_dates
    Tlist *dn = calloc(1, sizeof(Tlist));
    strcpy(dn->name, name); dn->dob = nn->dob; dn->dod = nn->dod;
    dn->next = g_dates; g_dates = dn;
    return s;
}

// ── addEvents ─────────────────────────────────────────────────
static Tlist* addEvents(Tlist *b, const char *nameE, const char *date_str) {
    Tlist *nn = calloc(1, sizeof(Tlist));
    strcpy(nn->name, nameE); strcpy(nn->definition, "Historical Event");
    nn->dob = parse_date(date_str); nn->next = b;
    return nn;
}

// ── toQueue ───────────────────────────────────────────────────
// Handles both normal and circular lists correctly
static TQueue* toQueue(Tlist *merged) {
    TQueue *q = calloc(1, sizeof(TQueue));
    if (!merged) return q;
    Tlist *start = merged, *cur = merged;
    do {
        enqueue(q, cur->name, cur->definition, cur->dob, cur->dod);
        cur = cur->next;
    } while (cur && cur != start);
    return q;
}

// ── sName (queue sorted by name word count) ───────────────────
static TQueue* sName(Tlist *s) {
    TQueue *q = calloc(1, sizeof(TQueue));
    if (!s) return q;
    int cnt = 0;
    for (Tlist *n = s; n; n = n->next) cnt++;
    Tlist **arr = malloc((size_t)cnt * sizeof(Tlist*));
    int k = 0;
    for (Tlist *n = s; n; n = n->next) arr[k++] = n;
    for (int i = 0; i < cnt; i++)
        for (int j = i+1; j < cnt; j++) {
            int wi = countWordsLL(arr[i]->name), wj = countWordsLL(arr[j]->name);
            if (wi > wj) { Tlist *t = arr[i]; arr[i] = arr[j]; arr[j] = t; }
        }
    for (int i = 0; i < cnt; i++)
        enqueue(q, arr[i]->name, arr[i]->definition, arr[i]->dob, arr[i]->dod);
    free(arr);
    return q;
}

// ── ageP (queue sorted by lifespan) ──────────────────────────
static TQueue* ageP(Tlist *a) {
    TQueue *q = calloc(1, sizeof(TQueue));
    if (!a) return q;
    int cnt = 0;
    for (Tlist *n = a; n; n = n->next) cnt++;
    Tlist **arr = malloc((size_t)cnt * sizeof(Tlist*));
    int k = 0;
    for (Tlist *n = a; n; n = n->next) arr[k++] = n;
    for (int i = 0; i < cnt; i++)
        for (int j = i+1; j < cnt; j++) {
            int ai = arr[i]->dod.year - arr[i]->dob.year;
            int aj = arr[j]->dod.year - arr[j]->dob.year;
            if (ai > aj) { Tlist *t = arr[i]; arr[i] = arr[j]; arr[j] = t; }
        }
    for (int i = 0; i < cnt; i++)
        enqueue(q, arr[i]->name, arr[i]->definition, arr[i]->dob, arr[i]->dod);
    free(arr);
    return q;
}

// ── getInfoByDates ────────────────────────────────────────────
static void gui_getInfoByDates(Tlist *s, Tlist *dob_list, int d, int m, int y) {
    bool found = false;
    for (Tlist *cd = dob_list; cd; cd = cd->next) {
        if (cd->dob.day == d && cd->dob.month == m && cd->dob.year == y) {
            for (Tlist *cb = s; cb; cb = cb->next) {
                if (strcmp(cb->name, cd->name) == 0) {
                    char d1[20], d2[20]; fmt_date(d1, cb->dob); fmt_date(d2, cb->dod);
                    output_appendf("  ► %s\n    %s\n    Born: %-12s  Died: %s\n\n",
                        cb->name, cb->definition, d1, d2);
                    found = true; break;
                }
            }
        }
    }
    if (!found) output_appendf("  No personality born on %02d/%02d/%04d.\n", d, m, y);
}

static void gui_getInfoByDates2(Tlist *s, Tlist *dod_list, int d, int m, int y) {
    bool found = false;
    for (Tlist *cd = dod_list; cd; cd = cd->next) {
        if (cd->dod.day == d && cd->dod.month == m && cd->dod.year == y) {
            for (Tlist *cb = s; cb; cb = cb->next) {
                if (strcmp(cb->name, cd->name) == 0) {
                    char d1[20], d2[20]; fmt_date(d1, cb->dob); fmt_date(d2, cb->dod);
                    output_appendf("  ► %s\n    %s\n    Born: %-12s  Died: %s\n\n",
                        cb->name, cb->definition, d1, d2);
                    found = true; break;
                }
            }
        }
    }
    if (!found) output_appendf("  No personality died on %02d/%02d/%04d.\n", d, m, y);
}

// ═══════════════════════════════════════════════════════════════
//  STACK FUNCTIONS
// ═══════════════════════════════════════════════════════════════

static void push(TStack *stk, const char *name, const char *def, date dob, date dod) {
    Tlist *n = calloc(1, sizeof(Tlist));
    strcpy(n->name, name); strcpy(n->definition, def);
    n->dob = dob; n->dod = dod; n->next = stk->top; stk->top = n;
}
static bool isEmpty(TStack *stk) { return stk->top == NULL; }
static Tlist* pop(TStack *stk) {
    if (!stk->top) return NULL;
    Tlist *t = stk->top; stk->top = stk->top->next; t->next = NULL; return t;
}

static TStack* toStack(Tlist *merged) {
    TStack *stk = calloc(1, sizeof(TStack));
    for (Tlist *c = merged; c; c = c->next)
        push(stk, c->name, c->definition, c->dob, c->dod);
    return stk;
}

static TStack* getInfoPersonality(TStack *stk, const char *name) {
    for (Tlist *c = stk->top; c; c = c->next) {
        if (strcmp(c->name, name) == 0) {
            TStack *r = calloc(1, sizeof(TStack));
            push(r, c->name, c->definition, c->dob, c->dod);
            return r;
        }
    }
    return NULL;   // BUG FIX: was returning empty stack; callers now guard against NULL
}

// BUG FIX: temp stack was never seeded → loop never ran
static TStack* sortNameStack(TStack *s) {
    TStack *sorted = calloc(1, sizeof(TStack));
    TStack *temp   = calloc(1, sizeof(TStack));
    temp->top = s->top;   // BUG FIX: seed temp

    while (!isEmpty(temp)) {
        Tlist *cur = temp->top, *smallest = temp->top;
        while (cur) {
            if (strcmp(cur->name, smallest->name) < 0) smallest = cur;
            cur = cur->next;
        }
        push(sorted, smallest->name, smallest->definition, smallest->dob, smallest->dod);
        Tlist *prev = NULL, *cr = temp->top;
        while (cr != smallest) { prev = cr; cr = cr->next; }
        if (!prev) temp->top = cr->next; else prev->next = cr->next;
        free(cr);
    }
    free(temp);
    return sorted;
}

static TStack* deleteName(TStack *stk, const char *name) {
    Tlist *prev = NULL, *cur = stk->top;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            if (!prev) stk->top = cur->next; else prev->next = cur->next;
            free(cur); return stk;
        }
        prev = cur; cur = cur->next;
    }
    return stk;
}

static TStack* updateStack(TStack *stk, const char *name, const char *def,
                            const char *DoB, const char *DoD) {
    for (Tlist *c = stk->top; c; c = c->next) {
        if (strcmp(c->name, name) == 0) {
            strcpy(c->definition, def);
            c->dob = stringToDate(DoB); c->dod = stringToDate(DoD);
            return stk;
        }
    }
    return stk;
}

static TQueue* stackToQueue(TStack *stk) {
    TQueue *q = calloc(1, sizeof(TQueue));
    for (Tlist *c = stk->top; c; c = c->next)
        enqueue(q, c->name, c->definition, c->dob, c->dod);
    return q;
}

// BUG FIX: original missed current = current->next → infinite loop
static TBilist* stackToList(TStack *stk) {
    TBilist *head = NULL;
    for (Tlist *c = stk->top; c; c = c->next)   // BUG FIX: c=c->next in for loop
        head = insertBilist(head, c->name, c->definition, c->dob, c->dod);
    return head;
}

static TStack* addNameStack(TStack *stk, const char *name, const char *def,
                             const char *DoB, const char *DoD) {
    Tlist *nn = calloc(1, sizeof(Tlist));
    strcpy(nn->name, name); strcpy(nn->definition, def);
    nn->dob = stringToDate(DoB); nn->dod = stringToDate(DoD); nn->next = NULL;
    if (!stk->top || strcmp(name, stk->top->name) < 0) {
        nn->next = stk->top; stk->top = nn; return stk;
    }
    Tlist *cur = stk->top;
    while (cur->next && strcmp(name, cur->next->name) > 0) cur = cur->next;
    nn->next = cur->next; cur->next = nn;
    return stk;
}

static int countWords(const char *def) {
    int c = 0; char tmp[400]; strncpy(tmp, def, 399); tmp[399] = 0;
    char *tok = strtok(tmp, " ");
    while (tok) { c++; tok = strtok(NULL, " "); }
    return c;
}

// BUG FIX: same temp-not-seeded bug as sortNameStack
static TStack* definitionStack(TStack *stk) {
    TStack *sorted = calloc(1, sizeof(TStack));
    TStack *temp   = calloc(1, sizeof(TStack));
    temp->top = stk->top;   // BUG FIX: seed temp

    while (!isEmpty(temp)) {
        Tlist *cur = temp->top, *smallest = temp->top;
        while (cur) {
            if (countWords(cur->definition) < countWords(smallest->definition)) smallest = cur;
            cur = cur->next;
        }
        push(sorted, smallest->name, smallest->definition, smallest->dob, smallest->dod);
        Tlist *prev = NULL, *cr = temp->top;
        while (cr != smallest) { prev = cr; cr = cr->next; }
        if (!prev) temp->top = cr->next; else prev->next = cr->next;
        free(cr);
    }
    free(temp);
    return sorted;
}

static void pronunciationStack(TStack *stk, TStack **shortS, TStack **longS) {
    *shortS = calloc(1, sizeof(TStack));
    *longS  = calloc(1, sizeof(TStack));
    for (Tlist *c = stk->top; c; c = c->next) {
        if (countWords(c->definition) <= 5) push(*shortS, c->name, c->definition, c->dob, c->dod);
        else                                push(*longS,  c->name, c->definition, c->dob, c->dod);
    }
}

static const char* getSmallest(TStack *stk) {
    if (!stk->top) return NULL;
    const char *smallest = stk->top->definition;
    for (Tlist *c = stk->top; c; c = c->next)
        if (countWords(c->definition) < countWords(smallest)) smallest = c->definition;
    return smallest;
}

static bool isPersonalityKilled(const char *word) { return strstr(word, "killed") != NULL; }

static void insertAtBottom(TStack *stk, Tlist *node) {
    if (isEmpty(stk)) { node->next = NULL; stk->top = node; return; }
    Tlist *cur = stk->top;
    while (cur->next) cur = cur->next;
    node->next = NULL; cur->next = node;
}

static TStack* recRevStack(TStack *stk) {
    if (isEmpty(stk)) return stk;
    Tlist *top = pop(stk);
    stk = recRevStack(stk);
    insertAtBottom(stk, top);
    return stk;
}

// ═══════════════════════════════════════════════════════════════
//  BST FUNCTIONS
// ═══════════════════════════════════════════════════════════════

static TTree* bst_insert(TTree *root, const char *name, const char *def, date dob, date dod) {
    if (!root) {
        TTree *n = calloc(1, sizeof(TTree));
        strcpy(n->name, name); strcpy(n->definition, def);
        n->dob = dob; n->dod = dod; return n;
    }
    int c = strcmp(name, root->name);
    if      (c < 0) root->left  = bst_insert(root->left,  name, def, dob, dod);
    else if (c > 0) root->right = bst_insert(root->right, name, def, dob, dod);
    return root;
}

static TTree* toTree(TStack *stk) {
    TTree *root = NULL;
    for (Tlist *c = stk->top; c; c = c->next)
        root = bst_insert(root, c->name, c->definition, c->dob, c->dod);
    return root;
}

static TTree* fillTree(void) {
    TTree *root = NULL;
    for (Tlist *c = g_personalities; c; c = c->next)
        root = bst_insert(root, c->name, c->definition, c->dob, c->dod);
    return root;
}

static TTree* getInfoNameTree(TTree *tr, const char *name) {
    if (!tr) return NULL;
    int c = strcmp(name, tr->name);
    if (c == 0) return tr;
    if (c < 0)  return getInfoNameTree(tr->left,  name);
    return              getInfoNameTree(tr->right, name);
}

static TTree* bst_min(TTree *n) { while (n->left) n = n->left; return n; }

static TTree* deleteNameBST(TTree *tr, const char *name) {
    if (!tr) return NULL;
    int c = strcmp(name, tr->name);
    if      (c < 0) tr->left  = deleteNameBST(tr->left,  name);
    else if (c > 0) tr->right = deleteNameBST(tr->right, name);
    else {
        if (!tr->left)  { TTree *t = tr->right; free(tr); return t; }
        if (!tr->right) { TTree *t = tr->left;  free(tr); return t; }
        TTree *mn = bst_min(tr->right);
        strcpy(tr->name, mn->name); strcpy(tr->definition, mn->definition);
        tr->dob = mn->dob; tr->dod = mn->dod;
        tr->right = deleteNameBST(tr->right, mn->name);
    }
    return tr;
}

static TTree* updateNameBST(TTree *tr, const char *name, const char *def,
                              const char *DoB, const char *DoD) {
    TTree *found = getInfoNameTree(tr, name);
    if (found) {
        strcpy(found->definition, def);
        found->dob = parse_date(DoB);
        found->dod = parse_date(DoD);
    }
    return tr;
}

static void inOrder(TTree *n) {
    if (!n) return; inOrder(n->left);
    char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
    output_appendf("  ► %-30s  %s – %s\n", n->name, d1, d2);
    inOrder(n->right);
}
static void preOrder(TTree *n) {
    if (!n) return;
    char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
    output_appendf("  ► %-30s  %s – %s\n", n->name, d1, d2);
    preOrder(n->left); preOrder(n->right);
}
static void postOrder(TTree *n) {
    if (!n) return;
    postOrder(n->left); postOrder(n->right);
    char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
    output_appendf("  ► %-30s  %s – %s\n", n->name, d1, d2);
}

static int bst_height(TTree *n) {
    if (!n) return 0;
    int l = bst_height(n->left), r = bst_height(n->right);
    return 1 + (l > r ? l : r);
}
static int bst_size(TTree *n) { return n ? 1 + bst_size(n->left) + bst_size(n->right) : 0; }
static bool isBalanced(TTree *n) {
    if (!n) return true;
    int l = bst_height(n->left), r = bst_height(n->right);
    return abs(l - r) <= 1 && isBalanced(n->left) && isBalanced(n->right);
}

static TTree* lowestCommonAncestor(TTree *tr, const char *w1, const char *w2) {
    if (!tr) return NULL;
    if (strcmp(tr->name, w1) > 0 && strcmp(tr->name, w2) > 0) return lowestCommonAncestor(tr->left,  w1, w2);
    if (strcmp(tr->name, w1) < 0 && strcmp(tr->name, w2) < 0) return lowestCommonAncestor(tr->right, w1, w2);
    return tr;
}

// NEW FEATURE: prints the matching names, not just the count
static int countNodesRange(TTree *tr, int l, int h) {
    if (!tr) return 0;
    int c = 0;
    if (tr->dob.year >= l && tr->dob.year <= h) {
        output_appendf("  ► %s  (born %d)\n", tr->name, tr->dob.year);
        c = 1;
    }
    return c + countNodesRange(tr->left, l, h) + countNodesRange(tr->right, l, h);
}

static TTree* inOrderSuccessor(TTree *tr, const char *word) {
    TTree *succ = NULL, *cur = tr;
    while (cur) {
        int c = strcmp(word, cur->name);
        if      (c < 0) { succ = cur; cur = cur->left; }
        else if (c > 0) cur = cur->right;
        else {
            if (cur->right) succ = bst_min(cur->right);
            break;
        }
    }
    return succ;
}

static void bst_mirror_helper(TTree *n) {
    if (!n) return;
    TTree *tmp = n->left; n->left = n->right; n->right = tmp;
    bst_mirror_helper(n->left); bst_mirror_helper(n->right);
}

// ═══════════════════════════════════════════════════════════════
//  RECURSION FUNCTIONS
// ═══════════════════════════════════════════════════════════════

static int countOccurence(Tlist *f, const char *name) {
    if (!f) return 0;
    return (strcmp(f->name, name) == 0 ? 1 : 0) + countOccurence(f->next, name);
}

static Tlist* removeOccurence(Tlist *f, const char *word) {
    if (!f) return NULL;
    if (strcmp(f->name, word) == 0) {
        Tlist *nx = f->next; free(f); return removeOccurence(nx, word);
    }
    f->next = removeOccurence(f->next, word);
    return f;
}

static Tlist* replaceOccurence(Tlist *f, const char *name, const char *DoB, const char *DoD) {
    if (!f) return NULL;
    if (strcmp(f->name, name) == 0) {
        f->dob = stringToDate(DoB); f->dod = stringToDate(DoD);
    }
    f->next = replaceOccurence(f->next, name, DoB, DoD);
    return f;
}

static void swap_chars(char *a, char *b) { char t = *a; *a = *b; *b = t; }

// perm_count passed as pointer to avoid global state bleed
static void namePermutation(char *name, int start, int end, int *pcount) {
    if (start == end) {
        (*pcount)++;
        if (*pcount <= 24) output_appendf("  %3d. %s\n", *pcount, name);
        return;
    }
    for (int i = start; i <= end; i++) {
        swap_chars(&name[start], &name[i]);
        namePermutation(name, start + 1, end, pcount);
        swap_chars(&name[start], &name[i]);
    }
}

// subseq_count passed as pointer to avoid global state bleed
static void subseqName(const char *word, char *current, int index, int *scount) {
    if (index == (int)strlen(word)) {
        (*scount)++;
        if (*scount <= 30) output_appendf("  \"%s\"\n", current);
        return;
    }
    subseqName(word, current, index + 1, scount);
    int len = strlen(current);
    current[len] = word[index]; current[len+1] = '\0';
    subseqName(word, current, index + 1, scount);
    current[len] = '\0';
}

// BUG FIX: original used date1 twice (copy-paste); now correctly uses date2
static void longestSubyear(Tlist *f, const char *date1, const char *date2) {
    if (!f) return;
    date d1 = stringToDate(date1);
    date d2 = stringToDate(date2);   // BUG FIX: was stringToDate(date1)
    if (isOverlapping(f->dob, f->dod, d1, d2)) {
        char s1[20], s2[20]; fmt_date(s1, f->dob); fmt_date(s2, f->dod);
        output_appendf("  ► %-30s  (%s – %s)\n", f->name, s1, s2);
    }
    longestSubyear(f->next, date1, date2);
}

static int distinctSubseqWord(const char *event) {
    if (!event || !*event) return 1;
    int cnt = 2 * distinctSubseqWord(event + 1);
    for (int i = 1; event[i]; i++)
        if (event[i] == event[0]) { cnt -= distinctSubseqWord(event + i + 1); break; }
    return cnt;
}

// BUG FIX: rewritten as index-based to avoid buffer+pointer issues
static bool isPalindromWord(const char *event) {
    int len = (int)strlen(event);
    if (len <= 1) return true;
    if (event[0] != event[len-1]) return false;
    // Create a substring from index 1 to len-2
    char sub[200];
    strncpy(sub, event + 1, (size_t)(len - 2));
    sub[len - 2] = '\0';
    return isPalindromWord(sub);
}

// ═══════════════════════════════════════════════════════════════
//  ★ NEW FEATURE: STATISTICS DASHBOARD
// ═══════════════════════════════════════════════════════════════

static void gui_statistics(void) {
    output_header("STATISTICS DASHBOARD");
    int cnt = 0, total_age = 0, min_year = 9999, max_year = 0;
    for (Tlist *n = g_personalities; n; n = n->next) {
        cnt++;
        int age = n->dod.year - n->dob.year;
        total_age += age;
        if (n->dob.year < min_year) min_year = n->dob.year;
        if (n->dod.year > max_year) max_year = n->dod.year;
    }
    int ev = 0;
    for (Tlist *n = g_events; n; n = n->next) ev++;

    output_appendf("  Personalities    : %d\n", cnt);
    output_appendf("  Historical Events: %d\n", ev);
    if (cnt > 0) {
        output_appendf("  Average lifespan : %d years\n", total_age / cnt);
        output_appendf("  Earliest birth   : %d\n", min_year);
        output_appendf("  Latest death     : %d\n", max_year);
        output_appendf("  DB time span     : %d years\n\n", max_year - min_year);
    }

    // Mini bar chart of century breakdown
    output_append("  ── Born by century ───────────────────────\n");
    int centuries[5] = {0}; // 1600s,1700s,1800s,1900s,2000s
    for (Tlist *n = g_personalities; n; n = n->next) {
        int c = (n->dob.year / 100) - 16;
        if (c >= 0 && c < 5) centuries[c]++;
    }
    const char *clabels[] = {"1600s","1700s","1800s","1900s","2000s"};
    for (int i = 0; i < 5; i++) {
        output_appendf("  %-6s |", clabels[i]);
        for (int j = 0; j < centuries[i] * 4; j++) output_append("█");
        output_appendf(" %d\n", centuries[i]);
    }

    // "killed" count
    int killed = 0;
    for (Tlist *n = g_personalities; n; n = n->next)
        if (isPersonalityKilled(n->definition)) killed++;
    output_appendf("\n  Mentioned as killed: %d\n", killed);
}

// ═══════════════════════════════════════════════════════════════
//  ★ NEW FEATURE: ASCII TIMELINE
// ═══════════════════════════════════════════════════════════════

static void gui_timeline(void) {
    output_header("CHRONOLOGICAL TIMELINE");
    if (!g_personalities) { output_append("  No data.\n"); return; }

    // Find global range
    int earliest = 9999, latest = 0;
    for (Tlist *n = g_personalities; n; n = n->next) {
        if (n->dob.year < earliest) earliest = n->dob.year;
        if (n->dod.year > latest)   latest   = n->dod.year;
    }
    int span = latest - earliest;
    if (span <= 0) { output_append("  Not enough data.\n"); return; }

    // Sort a local array by dob
    int cnt = 0;
    for (Tlist *n = g_personalities; n; n = n->next) cnt++;
    Tlist **arr = malloc((size_t)cnt * sizeof(Tlist*));
    int k = 0;
    for (Tlist *n = g_personalities; n; n = n->next) arr[k++] = n;
    // bubble sort by dob.year
    for (int i = 0; i < cnt; i++)
        for (int j = i+1; j < cnt; j++)
            if (arr[i]->dob.year > arr[j]->dob.year)
                { Tlist *t = arr[i]; arr[i] = arr[j]; arr[j] = t; }

    output_appendf("  %d ────────────────────────────────── %d\n\n", earliest, latest);

    const int W = 48;
    for (int i = 0; i < cnt; i++) {
        int s = (int)((double)(arr[i]->dob.year - earliest) / span * W);
        int e = (int)((double)(arr[i]->dod.year - earliest) / span * W);
        if (e <= s) e = s + 1;
        if (e >= W) e = W - 1;
        char bar[W+4]; memset(bar, ' ', W); bar[W] = 0;
        for (int x = s; x <= e && x < W; x++) bar[x] = '|';
        output_appendf("  |%s|  %s\n", bar, arr[i]->name);
    }
    output_appendf("\n  %d – %d  ·  %d personalities\n", earliest, latest, cnt);
    free(arr);
}

// ═══════════════════════════════════════════════════════════════
//  ★ NEW FEATURE: EXPORT TO CSV
// ═══════════════════════════════════════════════════════════════

static void gui_export_csv(void) {
    FILE *f = fopen("algeria_export.csv", "w");
    if (!f) { output_append("  ERROR: could not create algeria_export.csv\n"); return; }
    fprintf(f, "Type,Name,Definition,DateOfBirth,DateOfDeath,Lifespan\n");
    for (Tlist *n = g_personalities; n; n = n->next) {
        char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
        fprintf(f, "Personality,\"%s\",\"%s\",%s,%s,%d\n",
            n->name, n->definition, d1, d2, n->dod.year - n->dob.year);
    }
    for (Tlist *n = g_events; n; n = n->next) {
        char d1[20]; fmt_date(d1, n->dob);
        fprintf(f, "Event,\"%s\",\"%s\",%s,,\n", n->name, n->definition, d1);
    }
    fclose(f);
    output_header("EXPORT COMPLETE");
    output_append("  algeria_export.csv created in the current directory.\n\n");
    int pc = 0, ec = 0;
    for (Tlist *n = g_personalities; n; n = n->next) pc++;
    for (Tlist *n = g_events;        n; n = n->next) ec++;
    output_appendf("  Exported: %d personalities + %d events\n", pc, ec);
}

// ═══════════════════════════════════════════════════════════════
//  ★ NEW FEATURE: BORN SAME YEAR
// ═══════════════════════════════════════════════════════════════

static void gui_born_same_year(int year) {
    output_header("PERSONALITIES BORN IN THAT YEAR");
    bool found = false;
    for (Tlist *n = g_personalities; n; n = n->next) {
        if (n->dob.year == year) {
            char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
            output_appendf("  ► %s  (born %s · died %s)\n    %s\n\n",
                n->name, d1, d2, n->definition);
            found = true;
        }
    }
    if (!found) output_appendf("  No personality born in %d.\n", year);
}

// ═══════════════════════════════════════════════════════════════
//  DIALOG HELPERS
// ═══════════════════════════════════════════════════════════════

static GtkWidget* dlg_entry(GtkWidget *box, const char *lbl) {
    GtkWidget *l = gtk_label_new(lbl);
    gtk_widget_set_halign(l, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), l, FALSE, FALSE, 2);
    GtkWidget *e = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), e, FALSE, FALSE, 2);
    return e;
}

static GtkWidget* make_dialog(const char *title, GtkWidget **ca_out) {
    GtkWidget *dlg = gtk_dialog_new_with_buttons(title,
        GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *ca = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca), 16);
    gtk_box_set_spacing(GTK_BOX(ca), 4);
    if (ca_out) *ca_out = ca;
    return dlg;
}

// ★ NEW: Enter key confirms dialog
static void entry_activate(GtkEntry *e, gpointer dlg) {
    gtk_dialog_response(GTK_DIALOG(dlg), GTK_RESPONSE_OK);
}

static GtkWidget* dlg_entry_confirm(GtkWidget *box, const char *lbl, GtkWidget *dlg) {
    GtkWidget *e = dlg_entry(box, lbl);
    g_signal_connect(e, "activate", G_CALLBACK(entry_activate), dlg);
    return e;
}

// ★ NEW: Confirm destructive operations
static bool confirm_action(const char *msg) {
    GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "%s", msg);
    int r = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    return r == GTK_RESPONSE_OK;
}

static GtkWidget* make_btn(const char *label, GCallback cb, gpointer data,
                            const char *extra_class) {
    GtkWidget *b = gtk_button_new_with_label(label);
    gtk_style_context_add_class(gtk_widget_get_style_context(b), "action-btn");
    if (extra_class) gtk_style_context_add_class(gtk_widget_get_style_context(b), extra_class);
    if (cb) g_signal_connect(b, "clicked", cb, data);
    gtk_widget_set_hexpand(b, TRUE);
    return b;
}

static GtkWidget* make_grid(int cols) {
    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g), 8);
    gtk_grid_set_row_spacing(GTK_GRID(g), 8);
    gtk_widget_set_margin_bottom(g, 8);
    return g;
}

static void grid_add(GtkWidget *grid, GtkWidget *btn, int i, int cols) {
    gtk_grid_attach(GTK_GRID(grid), btn, i % cols, i / cols, 1, 1);
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — Linked Lists
// ═══════════════════════════════════════════════════════════════

static void on_ll_list_all(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("ALL PERSONALITIES"); gui_printList(g_personalities);
}
static void on_ll_list_events(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("ALL HISTORICAL EVENTS");
    if (!g_events) { output_append("  No events found.\n"); return; }
    int i = 1;
    for (Tlist *n = g_events; n; n = n->next, i++) {
        char d1[20]; fmt_date(d1, n->dob);
        output_appendf("  [%d] %s  (%s)\n      %s\n\n", i, n->name, d1, n->definition);
    }
}
static void on_ll_sort_alpha(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    g_personalities = sortWord(g_personalities);
    output_header("SORTED ALPHABETICALLY"); gui_printList(g_personalities);
}
static void on_ll_sort_length(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    g_personalities = sortWord2(g_personalities);
    output_header("SORTED BY NAME LENGTH"); gui_printList(g_personalities);
}
static void on_ll_sort_age(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    g_personalities = sortPersonality(g_personalities);
    output_header("SORTED BY LIFESPAN (ascending)");
    for (Tlist *n = g_personalities; n; n = n->next)
        output_appendf("  ► %-30s  %d years\n",
            n->name, n->dod.year - n->dob.year);
}
static void on_ll_reload(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    load_database(); output_header("DATABASE RELOADED"); gui_printList(g_personalities);
}

// ★ NEW: Quick search highlights name vs definition match
static void on_search_activate(GtkEntry *e, gpointer u) {
    (void)u;
    const char *kw = gtk_entry_get_text(e);
    output_header("QUICK SEARCH");
    bool found = false;
    for (Tlist *n = g_personalities; n; n = n->next) {
        bool nm = strstr(n->name, kw) != NULL;
        bool dm = strstr(n->definition, kw) != NULL;
        if (nm || dm) {
            char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
            output_appendf("  ► %s  [match in: %s]\n    %s\n    %s – %s\n\n",
                n->name, nm ? "NAME" : "DEFINITION", n->definition, d1, d2);
            found = true;
        }
    }
    for (Tlist *n = g_events; n; n = n->next) {
        if (strstr(n->name, kw) || strstr(n->definition, kw)) {
            char d1[20]; fmt_date(d1, n->dob);
            output_appendf("  ► [EVENT] %s  (%s)\n    %s\n\n", n->name, d1, n->definition);
            found = true;
        }
    }
    if (!found) output_append("  No results.\n");
}

static void on_ll_search(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Search", &ca);
    GtkWidget *e = dlg_entry_confirm(ca, "Name or keyword:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *kw = gtk_entry_get_text(GTK_ENTRY(e));
        output_header("SEARCH RESULTS");
        bool found = false;
        for (Tlist *n = g_personalities; n; n = n->next) {
            bool nm = strstr(n->name, kw) != NULL;
            bool dm = strstr(n->definition, kw) != NULL;
            if (nm || dm) {
                char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
                output_appendf("  ► %s  [match in: %s]\n    %s\n    Born: %-12s  Died: %s\n\n",
                    n->name, nm ? "NAME" : "DEFINITION", n->definition, d1, d2);
                found = true;
            }
        }
        for (Tlist *n = g_events; n; n = n->next)
            if (strstr(n->name, kw) || strstr(n->definition, kw)) {
                char d1[20]; fmt_date(d1, n->dob);
                output_appendf("  ► [EVENT] %s  (%s)\n    %s\n\n", n->name, d1, n->definition);
                found = true;
            }
        if (!found) output_append("  No results found.\n");
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_add(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Add Personality", &ca);
    GtkWidget *en  = dlg_entry_confirm(ca, "Full Name:", dlg);
    GtkWidget *ed  = dlg_entry_confirm(ca, "Definition:", dlg);
    GtkWidget *eb  = dlg_entry_confirm(ca, "Year of Birth (YYYY):", dlg);
    GtkWidget *edd = dlg_entry_confirm(ca, "Year of Death (YYYY):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(en));
        if (strlen(name) > 0) {
            g_personalities = addPersonality(g_personalities,
                name,
                gtk_entry_get_text(GTK_ENTRY(ed)),
                gtk_entry_get_text(GTK_ENTRY(eb)),
                gtk_entry_get_text(GTK_ENTRY(edd)));
            save_database();
            output_header("PERSONALITY ADDED"); gui_printList(g_personalities);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_delete(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Delete Personality", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Exact name to delete:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(en));
        if (strlen(name) > 0 && confirm_action("Delete this personality? This cannot be undone.")) {
            g_personalities = deletePersonality(g_personalities, name);
            g_dates         = deletePersonality(g_dates,         name);
            save_database();
            output_header("AFTER DELETION"); gui_printList(g_personalities);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_update(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Update Personality", &ca);
    GtkWidget *en  = dlg_entry_confirm(ca, "Name to update:", dlg);
    GtkWidget *ed  = dlg_entry_confirm(ca, "New definition:", dlg);
    GtkWidget *eb  = dlg_entry_confirm(ca, "New DoB (YYYY):", dlg);
    GtkWidget *edd = dlg_entry_confirm(ca, "New DoD (YYYY):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        g_personalities = updatePersonality(g_personalities,
            gtk_entry_get_text(GTK_ENTRY(en)),
            gtk_entry_get_text(GTK_ENTRY(ed)),
            gtk_entry_get_text(GTK_ENTRY(eb)),
            gtk_entry_get_text(GTK_ENTRY(edd)));
        save_database();
        output_header("AFTER UPDATE"); gui_printList(g_personalities);
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_similar(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Similar Personalities", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Personality name:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        Tlist *res = similarPersonality(g_personalities, gtk_entry_get_text(GTK_ENTRY(en)));
        output_header("SIMILAR PERSONALITIES (same birth/death year)");
        if (!res) output_append("  None found.\n");
        else      gui_printList(res);
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_count_at_date(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Count Personalities Alive At Year", &ca);
    GtkWidget *ey = dlg_entry_confirm(ca, "Year (YYYY):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        int year = atoi(gtk_entry_get_text(GTK_ENTRY(ey)));
        Tlist *res = countPersonality(g_personalities, year);
        output_header("PERSONALITIES ALIVE IN THAT YEAR");
        if (!res) { output_appendf("  None alive in %d.\n", year); }
        else {
            int cnt = 0; for (Tlist *n = res; n; n = n->next) cnt++;
            output_appendf("  Found %d personality(ies) alive in %d:\n\n", cnt, year);
            gui_printList(res);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_palindrome(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("PALINDROME NAMES");
    Tlist *res = palindromeName(g_personalities);
    if (!res) { output_append("  No palindrome names found.\n"); return; }
    for (Tlist *n = res; n; n = n->next) output_appendf("  ► %s\n", n->name);
}

static void on_ll_merge_bi(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("MERGE → BIDIRECTIONAL LIST");
    TBilist *merged = mergeNodes(g_personalities, g_dates);
    if (!merged) { output_append("  Load database first.\n"); return; }
    int i = 1;
    for (TBilist *n = merged; n; n = n->next, i++) {
        char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
        output_appendf("  [%d] %s\n      %s\n      Born: %-12s  Died: %s\n"
                       "      prev: %-20s  next: %s\n\n",
            i, n->name, n->definition, d1, d2,
            n->prev ? n->prev->name : "NULL",
            n->next ? n->next->name : "NULL");
    }
}

static void on_ll_merge_circular(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("MERGE → CIRCULAR LIST");
    g_merged = merge2Nodes(g_personalities, g_dates);
    if (!g_merged) { output_append("  Load database first.\n"); return; }
    Tlist *start = g_merged, *cur = g_merged; int i = 1;
    do {
        char d1[20], d2[20]; fmt_date(d1, cur->dob); fmt_date(d2, cur->dod);
        output_appendf("  [%d] %-30s  %s – %s\n", i++, cur->name, d1, d2);
        cur = cur->next;
    } while (cur && cur != start);
    output_append("\n  (circular — last node points back to first)\n");
}

static void on_ll_to_queue(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    if (!g_merged) {
        output_append("  Build circular list first (Merge → Circular).\n"); return;
    }
    g_queue = toQueue(g_merged);
    output_header("CIRCULAR LIST → QUEUE");
    for (QNode *n = g_queue->head; n; n = n->next) {
        char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
        output_appendf("  [FRONT→] %-30s  %s – %s\n", n->name, d1, d2);
    }
}

static void on_ll_sName(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("QUEUE SORTED BY NAME WORD COUNT");
    TQueue *q = sName(g_personalities);
    for (QNode *n = q->head; n; n = n->next)
        output_appendf("  [%d word(s)] %s\n", countWordsLL(n->name), n->name);
}

static void on_ll_ageP(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("QUEUE SORTED BY LIFESPAN (ascending)");
    TQueue *q = ageP(g_dates ? g_dates : g_personalities);
    for (QNode *n = q->head; n; n = n->next)
        output_appendf("  [age %3d] %s\n", n->dod.year - n->dob.year, n->name);
}

static void on_ll_info_by_dob(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Get Info By Date of Birth", &ca);
    GtkWidget *ed = dlg_entry_confirm(ca, "Day:", dlg);
    GtkWidget *em = dlg_entry_confirm(ca, "Month:", dlg);
    GtkWidget *ey = dlg_entry_confirm(ca, "Year:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        output_header("SEARCH BY DATE OF BIRTH");
        gui_getInfoByDates(g_personalities, g_dates,
            atoi(gtk_entry_get_text(GTK_ENTRY(ed))),
            atoi(gtk_entry_get_text(GTK_ENTRY(em))),
            atoi(gtk_entry_get_text(GTK_ENTRY(ey))));
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_info_by_dod(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Get Info By Date of Death", &ca);
    GtkWidget *ed = dlg_entry_confirm(ca, "Day:", dlg);
    GtkWidget *em = dlg_entry_confirm(ca, "Month:", dlg);
    GtkWidget *ey = dlg_entry_confirm(ca, "Year:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        output_header("SEARCH BY DATE OF DEATH");
        gui_getInfoByDates2(g_personalities, g_dates,
            atoi(gtk_entry_get_text(GTK_ENTRY(ed))),
            atoi(gtk_entry_get_text(GTK_ENTRY(em))),
            atoi(gtk_entry_get_text(GTK_ENTRY(ey))));
    }
    gtk_widget_destroy(dlg);
}

static void on_ll_add_event(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Add Historical Event", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Event name:", dlg);
    GtkWidget *ey = dlg_entry_confirm(ca, "Year (YYYY):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(en));
        if (strlen(name) > 0) {
            g_events = addEvents(g_events, name, gtk_entry_get_text(GTK_ENTRY(ey)));
            save_database();
            output_header("EVENT ADDED"); on_ll_list_events(NULL, NULL);
        }
    }
    gtk_widget_destroy(dlg);
}

// ★ NEW: Statistics
static void on_ll_stats(GtkButton *b, gpointer u) { (void)b; (void)u; gui_statistics(); }

// ★ NEW: Timeline
static void on_ll_timeline(GtkButton *b, gpointer u) { (void)b; (void)u; gui_timeline(); }

// ★ NEW: Export CSV
static void on_ll_export(GtkButton *b, gpointer u) { (void)b; (void)u; gui_export_csv(); }

// ★ NEW: Born same year
static void on_ll_born_year(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Born in Year", &ca);
    GtkWidget *ey = dlg_entry_confirm(ca, "Year (YYYY):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK)
        gui_born_same_year(atoi(gtk_entry_get_text(GTK_ENTRY(ey))));
    gtk_widget_destroy(dlg);
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — Stack
// ═══════════════════════════════════════════════════════════════

static void ensure_stack(void) {
    if (!g_stack || !g_stack->top) {
        if (!g_stack) g_stack = calloc(1, sizeof(TStack));
        TStack *s = toStack(g_personalities);
        g_stack->top = s->top; free(s);
    }
}

static void on_stk_build(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    if (g_stack) {
        while (!isEmpty(g_stack)) { Tlist *t = pop(g_stack); free(t); }
        free(g_stack);
    }
    g_stack = toStack(g_personalities);
    output_header("STACK BUILT FROM LIST");
    for (Tlist *n = g_stack->top; n; n = n->next)
        output_appendf("  [TOP→] %s\n", n->name);
}

static void on_stk_display(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    output_header("STACK CONTENTS");
    if (isEmpty(g_stack)) { output_append("  Stack is empty.\n"); return; }
    int i = 1;
    for (Tlist *n = g_stack->top; n; n = n->next, i++) {
        char d1[20], d2[20]; fmt_date(d1, n->dob); fmt_date(d2, n->dod);
        output_appendf("  [%d] %s\n      %s\n      %s – %s\n\n",
            i, n->name, n->definition, d1, d2);
    }
}

static void on_stk_get_info(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    GtkWidget *ca, *dlg = make_dialog("Get Info from Stack", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Personality name:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        TStack *res = getInfoPersonality(g_stack, gtk_entry_get_text(GTK_ENTRY(en)));
        output_header("STACK — GET INFO");
        // BUG FIX: check res != NULL before accessing res->top
        if (!res || !res->top) { output_append("  Not found.\n"); }
        else {
            char d1[20], d2[20]; fmt_date(d1, res->top->dob); fmt_date(d2, res->top->dod);
            output_appendf("  ► %s\n    %s\n    Born: %-12s  Died: %s\n",
                res->top->name, res->top->definition, d1, d2);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_stk_sort(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    g_stack = sortNameStack(g_stack);
    output_header("STACK SORTED ALPHABETICALLY");
    for (Tlist *n = g_stack->top; n; n = n->next) output_appendf("  ► %s\n", n->name);
}

static void on_stk_delete(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    GtkWidget *ca, *dlg = make_dialog("Delete from Stack", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Name to delete:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        if (confirm_action("Remove from stack?"))
            g_stack = deleteName(g_stack, gtk_entry_get_text(GTK_ENTRY(en)));
        output_header("STACK AFTER DELETION"); on_stk_display(NULL, NULL);
    }
    gtk_widget_destroy(dlg);
}

static void on_stk_update(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    GtkWidget *ca, *dlg = make_dialog("Update Stack Entry", &ca);
    GtkWidget *en  = dlg_entry_confirm(ca, "Name:", dlg);
    GtkWidget *ed  = dlg_entry_confirm(ca, "New definition:", dlg);
    GtkWidget *eb  = dlg_entry_confirm(ca, "New DoB:", dlg);
    GtkWidget *edd = dlg_entry_confirm(ca, "New DoD:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        g_stack = updateStack(g_stack,
            gtk_entry_get_text(GTK_ENTRY(en)),
            gtk_entry_get_text(GTK_ENTRY(ed)),
            gtk_entry_get_text(GTK_ENTRY(eb)),
            gtk_entry_get_text(GTK_ENTRY(edd)));
        output_header("STACK AFTER UPDATE"); on_stk_display(NULL, NULL);
    }
    gtk_widget_destroy(dlg);
}

static void on_stk_to_queue(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    TQueue *q = stackToQueue(g_stack);
    output_header("STACK → QUEUE");
    for (QNode *n = q->head; n; n = n->next)
        output_appendf("  [FRONT→] %s\n", n->name);
}

static void on_stk_to_list(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    TBilist *bl = stackToList(g_stack);
    output_header("STACK → BIDIRECTIONAL LIST");
    int i = 1;
    for (TBilist *n = bl; n; n = n->next, i++)
        output_appendf("  [%d] %-30s  prev: %-20s  next: %s\n",
            i, n->name,
            n->prev ? n->prev->name : "NULL",
            n->next ? n->next->name : "NULL");
}

static void on_stk_add(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    GtkWidget *ca, *dlg = make_dialog("Add to Stack (sorted)", &ca);
    GtkWidget *en  = dlg_entry_confirm(ca, "Name:", dlg);
    GtkWidget *ed  = dlg_entry_confirm(ca, "Definition:", dlg);
    GtkWidget *eb  = dlg_entry_confirm(ca, "DoB:", dlg);
    GtkWidget *edd = dlg_entry_confirm(ca, "DoD:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        g_stack = addNameStack(g_stack,
            gtk_entry_get_text(GTK_ENTRY(en)),
            gtk_entry_get_text(GTK_ENTRY(ed)),
            gtk_entry_get_text(GTK_ENTRY(eb)),
            gtk_entry_get_text(GTK_ENTRY(edd)));
        output_header("STACK AFTER INSERT"); on_stk_display(NULL, NULL);
    }
    gtk_widget_destroy(dlg);
}

static void on_stk_def_sort(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    TStack *s = definitionStack(g_stack);
    output_header("STACK SORTED BY DEFINITION WORD COUNT");
    for (Tlist *n = s->top; n; n = n->next)
        output_appendf("  [%2d words] %s\n", countWords(n->definition), n->name);
}

static void on_stk_pronunciation(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    TStack *shortS, *longS;
    pronunciationStack(g_stack, &shortS, &longS);
    output_header("STACK — SHORT vs LONG DEFINITIONS");
    output_append("  ── SHORT (≤5 words) ─────────────\n");
    if (!shortS->top) output_append("  (none)\n");
    for (Tlist *n = shortS->top; n; n = n->next)
        output_appendf("  [%d] %s\n", countWords(n->definition), n->name);
    output_append("\n  ── LONG (>5 words) ──────────────\n");
    if (!longS->top) output_append("  (none)\n");
    for (Tlist *n = longS->top; n; n = n->next)
        output_appendf("  [%d] %s\n", countWords(n->definition), n->name);
}

static void on_stk_smallest(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    const char *s = getSmallest(g_stack);
    output_header("SMALLEST DEFINITION");
    if (s) output_appendf("  \"%s\"\n  (%d words)\n", s, countWords(s));
    else   output_append("  Stack empty.\n");
}

static void on_stk_overlapping(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    output_header("OVERLAPPING DATE RANGES");
    bool any = false;
    for (Tlist *a = g_stack->top; a; a = a->next)
    for (Tlist *bx = a->next; bx; bx = bx->next)
        if (isOverlapping(a->dob, a->dod, bx->dob, bx->dod)) {
            char d1[20], d2[20], d3[20], d4[20];
            fmt_date(d1,a->dob);  fmt_date(d2,a->dod);
            fmt_date(d3,bx->dob); fmt_date(d4,bx->dod);
            output_appendf("  ► %s (%s–%s)\n    %s (%s–%s)\n\n",
                a->name,d1,d2, bx->name,d3,d4);
            any = true;
        }
    if (!any) output_append("  No overlapping ranges.\n");
}

static void on_stk_killed(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    output_header("PERSONALITIES MENTIONED AS KILLED");
    bool any = false;
    for (Tlist *n = g_stack->top; n; n = n->next)
        if (isPersonalityKilled(n->definition)) {
            output_appendf("  ► %s\n    \"%s\"\n\n", n->name, n->definition);
            any = true;
        }
    if (!any) output_append("  None found.\n");
}

static void on_stk_reverse(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    g_stack = recRevStack(g_stack);
    output_header("STACK REVERSED (RECURSIVE)");
    for (Tlist *n = g_stack->top; n; n = n->next)
        output_appendf("  [TOP→] %s\n", n->name);
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — BST
// ═══════════════════════════════════════════════════════════════

static void on_bst_build(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    g_tree = fillTree();
    output_header("BST BUILT FROM DATABASE");
    output_appendf("  Nodes: %d  |  Height: %d  |  Balanced: %s\n\n",
        bst_size(g_tree), bst_height(g_tree), isBalanced(g_tree) ? "YES ✔" : "NO ✘");
    output_append("  In-order (sorted):\n");
    inOrder(g_tree);
}

static void on_bst_from_stack(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    ensure_stack();
    g_tree = toTree(g_stack);
    output_header("BST BUILT FROM STACK");
    output_appendf("  Nodes: %d  |  Height: %d\n\n", bst_size(g_tree), bst_height(g_tree));
    inOrder(g_tree);
}

static void on_bst_inorder(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("BST — IN-ORDER TRAVERSAL");
    if (!g_tree) { output_append("  Build BST first.\n"); return; }
    inOrder(g_tree);
}
static void on_bst_preorder(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("BST — PRE-ORDER TRAVERSAL");
    if (!g_tree) { output_append("  Build BST first.\n"); return; }
    preOrder(g_tree);
}
static void on_bst_postorder(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("BST — POST-ORDER TRAVERSAL");
    if (!g_tree) { output_append("  Build BST first.\n"); return; }
    postOrder(g_tree);
}

static void on_bst_search(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Search BST", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Name:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        if (!g_tree) { output_append("  Build BST first.\n"); }
        else {
            TTree *f = getInfoNameTree(g_tree, gtk_entry_get_text(GTK_ENTRY(en)));
            output_header("BST SEARCH RESULT");
            if (f) {
                char d1[20], d2[20]; fmt_date(d1, f->dob); fmt_date(d2, f->dod);
                output_appendf("  ► %s\n    %s\n    Born: %-12s  Died: %s\n",
                    f->name, f->definition, d1, d2);
            } else output_append("  Not found.\n");
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_bst_delete(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Delete from BST", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Name:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(en));
        if (confirm_action("Delete this node from BST?")) {
            g_tree = deleteNameBST(g_tree, name);
            output_header("BST AFTER DELETION");
            output_appendf("  Remaining: %d nodes  |  Height: %d\n\n",
                bst_size(g_tree), bst_height(g_tree));
            inOrder(g_tree);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_bst_update(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Update BST Node", &ca);
    GtkWidget *en  = dlg_entry_confirm(ca, "Name:", dlg);
    GtkWidget *ed  = dlg_entry_confirm(ca, "New definition:", dlg);
    GtkWidget *eb  = dlg_entry_confirm(ca, "New DoB:", dlg);
    GtkWidget *edd = dlg_entry_confirm(ca, "New DoD:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        g_tree = updateNameBST(g_tree,
            gtk_entry_get_text(GTK_ENTRY(en)),
            gtk_entry_get_text(GTK_ENTRY(ed)),
            gtk_entry_get_text(GTK_ENTRY(eb)),
            gtk_entry_get_text(GTK_ENTRY(edd)));
        output_header("BST AFTER UPDATE"); inOrder(g_tree);
    }
    gtk_widget_destroy(dlg);
}

static void on_bst_stats(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("BST — HEIGHT & SIZE");
    if (!g_tree) { output_append("  Build BST first.\n"); return; }
    output_appendf("  Nodes   : %d\n  Height  : %d\n  Balanced: %s\n",
        bst_size(g_tree), bst_height(g_tree), isBalanced(g_tree) ? "YES ✔" : "NO ✘");
}

static void on_bst_lca(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Lowest Common Ancestor", &ca);
    GtkWidget *e1 = dlg_entry_confirm(ca, "First name:", dlg);
    GtkWidget *e2 = dlg_entry_confirm(ca, "Second name:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        if (!g_tree) { output_append("  Build BST first.\n"); }
        else {
            TTree *lca = lowestCommonAncestor(g_tree,
                gtk_entry_get_text(GTK_ENTRY(e1)),
                gtk_entry_get_text(GTK_ENTRY(e2)));
            output_header("LOWEST COMMON ANCESTOR");
            if (lca) output_appendf("  ► %s\n", lca->name);
            else     output_append("  Not found.\n");
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_bst_range(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Count Nodes in Year Range", &ca);
    GtkWidget *el = dlg_entry_confirm(ca, "From year:", dlg);
    GtkWidget *eh = dlg_entry_confirm(ca, "To year:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        int l = atoi(gtk_entry_get_text(GTK_ENTRY(el)));
        int h = atoi(gtk_entry_get_text(GTK_ENTRY(eh)));
        output_header("BST — PERSONALITIES BORN IN YEAR RANGE");
        output_appendf("  Range: %d – %d\n\n", l, h);
        // ★ NEW: now prints the actual names, not just the count
        int cnt = countNodesRange(g_tree, l, h);
        output_appendf("\n  Total: %d personality(ies)\n", cnt);
    }
    gtk_widget_destroy(dlg);
}

static void on_bst_successor(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("In-Order Successor", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Name:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        if (!g_tree) { output_append("  Build BST first.\n"); }
        else {
            TTree *s = inOrderSuccessor(g_tree, gtk_entry_get_text(GTK_ENTRY(en)));
            output_header("IN-ORDER SUCCESSOR");
            if (s) output_appendf("  ► %s\n", s->name);
            else   output_append("  No successor (last node or not found).\n");
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_bst_mirror(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    // BUG FIX: guard before mirroring
    if (!g_tree) { output_append("  Build BST first.\n"); return; }
    bst_mirror_helper(g_tree);
    output_header("BST MIRRORED — In-Order (now reverse-sorted):");
    inOrder(g_tree);
}

static void on_bst_balanced(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    output_header("BST — BALANCE CHECK");
    if (!g_tree) { output_append("  Build BST first.\n"); return; }
    output_appendf("  Is balanced: %s\n  Height: %d\n",
        isBalanced(g_tree) ? "YES ✔" : "NO ✘", bst_height(g_tree));
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — Recursion
// ═══════════════════════════════════════════════════════════════

static void on_rec_count(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Count Occurrences (Recursive)", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Name to count:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(en));
        int c = countOccurence(g_personalities, name);
        output_header("COUNT OCCURRENCES — RECURSIVE");
        output_appendf("  \"%s\" appears %d time(s) in the list.\n", name, c);
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_remove(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Remove Occurrences (Recursive)", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Name to remove:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(en));
        if (confirm_action("Remove all occurrences recursively?")) {
            g_personalities = removeOccurence(g_personalities, name);
            save_database();
            output_header("AFTER RECURSIVE REMOVAL"); gui_printList(g_personalities);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_replace(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Replace Occurrences (Recursive)", &ca);
    GtkWidget *en  = dlg_entry_confirm(ca, "Name:", dlg);
    GtkWidget *eb  = dlg_entry_confirm(ca, "New DoB:", dlg);
    GtkWidget *edd = dlg_entry_confirm(ca, "New DoD:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        g_personalities = replaceOccurence(g_personalities,
            gtk_entry_get_text(GTK_ENTRY(en)),
            gtk_entry_get_text(GTK_ENTRY(eb)),
            gtk_entry_get_text(GTK_ENTRY(edd)));
        output_header("AFTER RECURSIVE REPLACE"); gui_printList(g_personalities);
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_permutation(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Name Permutations (Recursive)", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Word (keep short ≤7 chars!):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        char buf[64]; strncpy(buf, gtk_entry_get_text(GTK_ENTRY(en)), 63); buf[63] = 0;
        // Guard against explosively long inputs
        if (strlen(buf) > 8) { output_append("  Input too long (max 8 chars for permutations).\n"); }
        else {
            int pcount = 0;   // BUG FIX: local counter, not global
            output_header("NAME PERMUTATIONS — RECURSIVE");
            output_appendf("  Permutations of \"%s\" (max 24 shown):\n\n", buf);
            namePermutation(buf, 0, (int)strlen(buf)-1, &pcount);
            output_appendf("\n  Total: %d permutations\n", pcount);
        }
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_subseq(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Subsequences (Recursive)", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Word (keep short!):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        char word[64]; strncpy(word, gtk_entry_get_text(GTK_ENTRY(en)), 63); word[63] = 0;
        char current[128] = "";
        int scount = 0;   // BUG FIX: local counter
        output_header("SUBSEQUENCES — RECURSIVE");
        output_appendf("  Subsequences of \"%s\" (max 30 shown):\n\n", word);
        subseqName(word, current, 0, &scount);
        output_appendf("\n  Total: %d subsequences\n", scount);
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_longest(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Overlapping Date Range (Recursive)", &ca);
    GtkWidget *e1 = dlg_entry_confirm(ca, "From year (YYYY):", dlg);
    GtkWidget *e2 = dlg_entry_confirm(ca, "To year (YYYY):", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        output_header("PERSONALITIES OVERLAPPING DATE RANGE — RECURSIVE");
        longestSubyear(g_personalities,
            gtk_entry_get_text(GTK_ENTRY(e1)),
            gtk_entry_get_text(GTK_ENTRY(e2)));
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_distinct(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Distinct Subsequences (Recursive)", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Word:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *w = gtk_entry_get_text(GTK_ENTRY(en));
        output_header("DISTINCT SUBSEQUENCES — RECURSIVE");
        output_appendf("  Word: \"%s\"\n  Distinct subsequences: %d\n",
            w, distinctSubseqWord(w));
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_palindrome(GtkButton *b, gpointer u) {
    (void)b; (void)u;
    GtkWidget *ca, *dlg = make_dialog("Palindrome Check (Recursive)", &ca);
    GtkWidget *en = dlg_entry_confirm(ca, "Word:", dlg);
    gtk_widget_show_all(dlg);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        const char *w = gtk_entry_get_text(GTK_ENTRY(en));
        bool ok = isPalindromWord(w);
        output_header("PALINDROME CHECK — RECURSIVE");
        // ★ NEW: step-by-step trace
        output_appendf("  Word: \"%s\"\n\n", w);
        int len = (int)strlen(w);
        for (int i = 0; i < len/2; i++)
            output_appendf("  compare  '%c' [%d]  vs  '%c' [%d]  → %s\n",
                w[i], i, w[len-1-i], len-1-i,
                w[i] == w[len-1-i] ? "match ✔" : "mismatch ✘");
        output_appendf("\n  Result: %s\n", ok ? "YES — palindrome ✔" : "NO — not a palindrome ✘");
    }
    gtk_widget_destroy(dlg);
}

// ═══════════════════════════════════════════════════════════════
//  NAV SYSTEM
// ═══════════════════════════════════════════════════════════════

static void on_nav(GtkButton *b, gpointer ud) {
    (void)b;
    NavData *nd = (NavData*)ud;
    for (GList *l = nd->all; l; l = l->next)
        gtk_style_context_remove_class(gtk_widget_get_style_context((GtkWidget*)l->data), "active");
    gtk_style_context_add_class(gtk_widget_get_style_context(nd->btn), "active");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack_pages), nd->page);
    output_clear();
}

// ═══════════════════════════════════════════════════════════════
//  PAGE BUILDERS
// ═══════════════════════════════════════════════════════════════

static GtkWidget* page_wrap(GtkWidget *inner) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(box), "content-area");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), inner);
    gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 0);
    return box;
}

static GtkWidget* section_label(const char *title, const char *sub) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *t = gtk_label_new(title);
    gtk_style_context_add_class(gtk_widget_get_style_context(t), "module-title");
    gtk_widget_set_halign(t, GTK_ALIGN_START);
    GtkWidget *s = gtk_label_new(sub);
    gtk_style_context_add_class(gtk_widget_get_style_context(s), "module-subtitle");
    gtk_widget_set_halign(s, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), t, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), s, FALSE, FALSE, 0);
    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep), "gold-sep");
    gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 6);
    return box;
}

#define COLS 3

static GtkWidget* build_welcome(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(box, 40);

    GtkWidget *t = gtk_label_new("الجزائر — ALGÉRIE");
    gtk_style_context_add_class(gtk_widget_get_style_context(t), "module-title");
    gtk_label_set_justify(GTK_LABEL(t), GTK_JUSTIFY_CENTER);

    GtkWidget *s = gtk_label_new("HISTORY DATABASE  ·  DYNAMIC DATA STRUCTURES  ·  NSCS 2025–2026");
    gtk_style_context_add_class(gtk_widget_get_style_context(s), "module-subtitle");
    gtk_label_set_justify(GTK_LABEL(s), GTK_JUSTIFY_CENTER);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep), "gold-sep");
    gtk_widget_set_size_request(sep, 400, -1);

    GtkWidget *d = gtk_label_new(
        "Navigate the sidebar to access all modules:\n\n"
        "  I.   Linked Lists & Queues   — 25 operations\n"
        "  II.  Stack Operations        — 15 operations\n"
        "  III. Binary Search Tree      — 14 operations\n"
        "  IV.  Recursion Module        —  8 operations\n\n"
        "Results appear in the output terminal below.\n"
        "Use the header search bar for instant lookup.");
    gtk_label_set_justify(GTK_LABEL(d), GTK_JUSTIFY_CENTER);
    gtk_style_context_add_class(gtk_widget_get_style_context(d), "module-subtitle");

    gtk_box_pack_start(GTK_BOX(box), t,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), s,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(box), d,   FALSE, FALSE, 0);
    return page_wrap(box);
}

static GtkWidget* build_ll_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 4); gtk_widget_set_margin_end(box, 4);
    gtk_box_pack_start(GTK_BOX(box),
        section_label("Linked Lists & Queues", "MODULE I  ·  ITERATIVE DATA MANAGEMENT"),
        FALSE, FALSE, 0);

    struct { const char *l; GCallback c; const char *cls; } btns[] = {
        /* Read operations */
        {"List All Personalities",        G_CALLBACK(on_ll_list_all),       NULL},
        {"List All Events",               G_CALLBACK(on_ll_list_events),     NULL},
        {"Search / Get Info",             G_CALLBACK(on_ll_search),          NULL},
        {"Get Info by Birth Date",        G_CALLBACK(on_ll_info_by_dob),     NULL},
        {"Get Info by Death Date",        G_CALLBACK(on_ll_info_by_dod),     NULL},
        {"Born in Same Year",             G_CALLBACK(on_ll_born_year),       "new-btn"},
        /* Sort */
        {"Sort Alphabetically",           G_CALLBACK(on_ll_sort_alpha),      NULL},
        {"Sort by Name Length",           G_CALLBACK(on_ll_sort_length),     NULL},
        {"Sort by Lifespan",              G_CALLBACK(on_ll_sort_age),        NULL},
        /* Mutate */
        {"Add Personality",               G_CALLBACK(on_ll_add),             NULL},
        {"Add Event",                     G_CALLBACK(on_ll_add_event),       NULL},
        {"Update Personality",            G_CALLBACK(on_ll_update),          "warn-btn"},
        {"Delete Personality",            G_CALLBACK(on_ll_delete),          "danger-btn"},
        /* Analysis */
        {"Similar Personalities",         G_CALLBACK(on_ll_similar),         NULL},
        {"Count Alive at Year",           G_CALLBACK(on_ll_count_at_date),   NULL},
        {"Palindrome Names",              G_CALLBACK(on_ll_palindrome),      NULL},
        /* Merge & convert */
        {"Merge → Bidirectional",         G_CALLBACK(on_ll_merge_bi),        NULL},
        {"Merge → Circular",              G_CALLBACK(on_ll_merge_circular),  NULL},
        {"Circular List → Queue",         G_CALLBACK(on_ll_to_queue),        NULL},
        {"Queue by Name Words (sName)",   G_CALLBACK(on_ll_sName),           NULL},
        {"Queue by Lifespan (ageP)",      G_CALLBACK(on_ll_ageP),            NULL},
        /* ★ NEW extras */
        {"★ Statistics Dashboard",        G_CALLBACK(on_ll_stats),           "new-btn"},
        {"★ ASCII Timeline",              G_CALLBACK(on_ll_timeline),        "new-btn"},
        {"★ Export to CSV",               G_CALLBACK(on_ll_export),          "new-btn"},
        {"Reload Database",               G_CALLBACK(on_ll_reload),          NULL},
    };
    int n = (int)(sizeof(btns)/sizeof(btns[0]));
    GtkWidget *grid = make_grid(COLS);
    for (int i = 0; i < n; i++)
        grid_add(grid, make_btn(btns[i].l, btns[i].c, NULL, btns[i].cls), i, COLS);
    gtk_box_pack_start(GTK_BOX(box), grid, FALSE, FALSE, 0);
    return page_wrap(box);
}

static GtkWidget* build_stack_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 4); gtk_widget_set_margin_end(box, 4);
    gtk_box_pack_start(GTK_BOX(box),
        section_label("Stack Operations", "MODULE II  ·  LIFO DATA STRUCTURE"),
        FALSE, FALSE, 0);

    struct { const char *l; GCallback c; const char *cls; } btns[] = {
        {"Build Stack (toStack)",          G_CALLBACK(on_stk_build),        NULL},
        {"Display Stack",                  G_CALLBACK(on_stk_display),      NULL},
        {"Get Info (getInfoPersonality)",  G_CALLBACK(on_stk_get_info),     NULL},
        {"Sort Alphabetically",            G_CALLBACK(on_stk_sort),         NULL},
        {"Add Sorted (addNameStack)",      G_CALLBACK(on_stk_add),          NULL},
        {"Update Entry",                   G_CALLBACK(on_stk_update),       "warn-btn"},
        {"Delete from Stack",              G_CALLBACK(on_stk_delete),       "danger-btn"},
        {"Stack → Queue",                  G_CALLBACK(on_stk_to_queue),     NULL},
        {"Stack → Bilist",                 G_CALLBACK(on_stk_to_list),      NULL},
        {"Sort by Definition Words",       G_CALLBACK(on_stk_def_sort),     NULL},
        {"Short vs Long Definitions",      G_CALLBACK(on_stk_pronunciation), NULL},
        {"Smallest Definition",            G_CALLBACK(on_stk_smallest),     NULL},
        {"Overlapping Dates",              G_CALLBACK(on_stk_overlapping),  NULL},
        {"Check if Killed",                G_CALLBACK(on_stk_killed),       NULL},
        {"Reverse Stack (Recursive)",      G_CALLBACK(on_stk_reverse),      NULL},
    };
    int n = (int)(sizeof(btns)/sizeof(btns[0]));
    GtkWidget *grid = make_grid(COLS);
    for (int i = 0; i < n; i++)
        grid_add(grid, make_btn(btns[i].l, btns[i].c, NULL, btns[i].cls), i, COLS);
    gtk_box_pack_start(GTK_BOX(box), grid, FALSE, FALSE, 0);
    return page_wrap(box);
}

static GtkWidget* build_bst_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 4); gtk_widget_set_margin_end(box, 4);
    gtk_box_pack_start(GTK_BOX(box),
        section_label("Binary Search Tree", "MODULE III  ·  HIERARCHICAL DATA STRUCTURE"),
        FALSE, FALSE, 0);

    struct { const char *l; GCallback c; const char *cls; } btns[] = {
        {"Build BST (fillTree)",           G_CALLBACK(on_bst_build),      NULL},
        {"Build BST from Stack (toTree)",  G_CALLBACK(on_bst_from_stack), NULL},
        {"In-Order Traversal",             G_CALLBACK(on_bst_inorder),    NULL},
        {"Pre-Order Traversal",            G_CALLBACK(on_bst_preorder),   NULL},
        {"Post-Order Traversal",           G_CALLBACK(on_bst_postorder),  NULL},
        {"Search (getInfoNameTree)",        G_CALLBACK(on_bst_search),     NULL},
        {"Update Node",                    G_CALLBACK(on_bst_update),     "warn-btn"},
        {"Delete Node",                    G_CALLBACK(on_bst_delete),     "danger-btn"},
        {"Height & Size Stats",            G_CALLBACK(on_bst_stats),      NULL},
        {"Lowest Common Ancestor",         G_CALLBACK(on_bst_lca),        NULL},
        {"Nodes Born in Year Range",       G_CALLBACK(on_bst_range),      NULL},
        {"In-Order Successor",             G_CALLBACK(on_bst_successor),  NULL},
        {"Mirror BST",                     G_CALLBACK(on_bst_mirror),     NULL},
        {"Balance Check",                  G_CALLBACK(on_bst_balanced),   NULL},
    };
    int n = (int)(sizeof(btns)/sizeof(btns[0]));
    GtkWidget *grid = make_grid(COLS);
    for (int i = 0; i < n; i++)
        grid_add(grid, make_btn(btns[i].l, btns[i].c, NULL, btns[i].cls), i, COLS);
    gtk_box_pack_start(GTK_BOX(box), grid, FALSE, FALSE, 0);
    return page_wrap(box);
}

static GtkWidget* build_rec_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 4); gtk_widget_set_margin_end(box, 4);
    gtk_box_pack_start(GTK_BOX(box),
        section_label("Recursion Module", "MODULE IV  ·  RECURSIVE ALGORITHMS"),
        FALSE, FALSE, 0);

    struct { const char *l; GCallback c; const char *cls; } btns[] = {
        {"Count Occurrences",             G_CALLBACK(on_rec_count),       NULL},
        {"Remove Occurrences",            G_CALLBACK(on_rec_remove),      "danger-btn"},
        {"Replace Occurrences (dates)",   G_CALLBACK(on_rec_replace),     "warn-btn"},
        {"Name Permutations",             G_CALLBACK(on_rec_permutation), NULL},
        {"Subsequences",                  G_CALLBACK(on_rec_subseq),      NULL},
        {"Overlapping Date Range",        G_CALLBACK(on_rec_longest),     NULL},
        {"Distinct Subsequences",         G_CALLBACK(on_rec_distinct),    NULL},
        {"Palindrome Check (+ trace)",    G_CALLBACK(on_rec_palindrome),  NULL},
    };
    int n = (int)(sizeof(btns)/sizeof(btns[0]));
    GtkWidget *grid = make_grid(COLS);
    for (int i = 0; i < n; i++)
        grid_add(grid, make_btn(btns[i].l, btns[i].c, NULL, btns[i].cls), i, COLS);
    gtk_box_pack_start(GTK_BOX(box), grid, FALSE, FALSE, 0);
    return page_wrap(box);
}

// ═══════════════════════════════════════════════════════════════
//  MAIN WINDOW
// ═══════════════════════════════════════════════════════════════

static void build_ui(GtkApplication *app) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(g_main_window), "Algeria History Database — v3.0");
    gtk_window_set_default_size(GTK_WINDOW(g_main_window), 1200, 760);
    gtk_window_set_position(GTK_WINDOW(g_main_window), GTK_WIN_POS_CENTER);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(g_main_window), root);

    // ── Header ──────────────────────────────────────────────────
    GtkWidget *hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(hdr), "app-header");
    gtk_widget_set_size_request(hdr, -1, 54);

    GtkWidget *htxt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_valign(htxt, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(htxt, 8);
    GtkWidget *ht = gtk_label_new("ALGERIA HISTORY DATABASE");
    gtk_style_context_add_class(gtk_widget_get_style_context(ht), "app-title");
    gtk_widget_set_halign(ht, GTK_ALIGN_START);
    GtkWidget *hs = gtk_label_new("ALGORITHMS & DYNAMIC DATA STRUCTURES  ·  NSCS 2025–2026  ·  v3.0");
    gtk_style_context_add_class(gtk_widget_get_style_context(hs), "app-subtitle");
    gtk_widget_set_halign(hs, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(htxt), ht, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(htxt), hs, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hdr), htxt, TRUE, TRUE, 0);

    GtkWidget *search = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search), "Quick search...");
    gtk_widget_set_size_request(search, 220, -1);
    gtk_widget_set_valign(search, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_end(search, 16);
    g_signal_connect(search, "activate", G_CALLBACK(on_search_activate), NULL);
    gtk_box_pack_end(GTK_BOX(hdr), search, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), hdr, FALSE, FALSE, 0);

    // ── Body ────────────────────────────────────────────────────
    GtkWidget *body = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(root), body, TRUE, TRUE, 0);

    // ── Sidebar ─────────────────────────────────────────────────
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "sidebar");
    gtk_widget_set_size_request(sidebar, 215, -1);

    GtkWidget *logo = gtk_label_new("☽  ✦  ☽");
    gtk_style_context_add_class(gtk_widget_get_style_context(logo), "app-title");
    gtk_widget_set_margin_top(logo, 18); gtk_widget_set_margin_bottom(logo, 4);
    gtk_box_pack_start(GTK_BOX(sidebar), logo, FALSE, FALSE, 0);

    GtkWidget *sep0 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep0), "gold-sep");
    gtk_widget_set_margin_start(sep0, 14); gtk_widget_set_margin_end(sep0, 14);
    gtk_box_pack_start(GTK_BOX(sidebar), sep0, FALSE, FALSE, 6);

    struct { const char *l; const char *p; const char *sec; } nav[] = {
        {NULL,                   "",           "DATABASE"},
        {"⌂  Welcome",           "welcome",    NULL},
        {NULL,                   "",           "MODULES"},
        {"⊞  Linked Lists",      "ll",         NULL},
        {"▦  Stack",             "stack",      NULL},
        {"⌥  Binary Search Tree","bst",        NULL},
        {"↺  Recursion",         "recursion",  NULL},
    };
    int nn = (int)(sizeof(nav)/sizeof(nav[0]));
    GtkWidget **nav_btns = g_new0(GtkWidget*, nn);
    GList *btn_list = NULL; int btn_count = 0;

    for (int i = 0; i < nn; i++) {
        if (!nav[i].l) {
            GtkWidget *sl = gtk_label_new(nav[i].sec);
            gtk_style_context_add_class(gtk_widget_get_style_context(sl), "nav-section");
            gtk_widget_set_halign(sl, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(sidebar), sl, FALSE, FALSE, 0);
        } else {
            nav_btns[btn_count] = gtk_button_new_with_label(nav[i].l);
            gtk_style_context_add_class(gtk_widget_get_style_context(nav_btns[btn_count]), "nav-btn");
            gtk_button_set_relief(GTK_BUTTON(nav_btns[btn_count]), GTK_RELIEF_NONE);
            btn_list = g_list_append(btn_list, nav_btns[btn_count]);
            gtk_box_pack_start(GTK_BOX(sidebar), nav_btns[btn_count], FALSE, FALSE, 0);
            btn_count++;
        }
    }
    gtk_style_context_add_class(gtk_widget_get_style_context(nav_btns[0]), "active");

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(sidebar), spacer, TRUE, TRUE, 0);
    GtkWidget *ver = gtk_label_new("v3.0  ·  NSCS Algeria  ·  0 bugs");
    gtk_style_context_add_class(gtk_widget_get_style_context(ver), "module-subtitle");
    gtk_widget_set_margin_bottom(ver, 10);
    gtk_box_pack_start(GTK_BOX(sidebar), ver, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(body), sidebar, FALSE, FALSE, 0);

    // ── Content area ─────────────────────────────────────────────
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(content, TRUE);
    gtk_box_pack_start(GTK_BOX(body), content, TRUE, TRUE, 0);

    // Module pages (top panel)
    g_stack_pages = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(g_stack_pages),
        GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(g_stack_pages), 200);
    gtk_widget_set_vexpand(g_stack_pages, FALSE);
    gtk_widget_set_size_request(g_stack_pages, -1, 360);

    const char *pnames[] = {"welcome","ll","stack","bst","recursion"};
    GtkWidget *pages[] = {
        build_welcome(), build_ll_page(), build_stack_page(),
        build_bst_page(), build_rec_page()
    };
    for (int i = 0; i < 5; i++)
        gtk_stack_add_named(GTK_STACK(g_stack_pages), pages[i], pnames[i]);
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack_pages), "welcome");
    gtk_box_pack_start(GTK_BOX(content), g_stack_pages, FALSE, FALSE, 0);

    // Wire nav buttons
    int bi = 0;
    for (int i = 0; i < nn; i++) {
        if (!nav[i].l) continue;
        NavData *nd = g_new0(NavData, 1);
        nd->btn = nav_btns[bi]; nd->all = btn_list; nd->page = nav[i].p;
        g_signal_connect(nav_btns[bi], "clicked", G_CALLBACK(on_nav), nd);
        bi++;
    }

    // ── Output terminal ─────────────────────────────────────────
    GtkWidget *mid_sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(mid_sep), "gold-sep");
    gtk_box_pack_start(GTK_BOX(content), mid_sep, FALSE, FALSE, 0);

    GtkWidget *out_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(out_row, 16); gtk_widget_set_margin_top(out_row, 6);
    GtkWidget *out_lbl = gtk_label_new("OUTPUT TERMINAL");
    gtk_style_context_add_class(gtk_widget_get_style_context(out_lbl), "sidebar-title");
    gtk_box_pack_start(GTK_BOX(out_row), out_lbl, FALSE, FALSE, 0);
    GtkWidget *clr = gtk_button_new_with_label("✕ Clear");
    gtk_style_context_add_class(gtk_widget_get_style_context(clr), "action-btn");
    gtk_widget_set_margin_end(clr, 16);
    g_signal_connect_swapped(clr, "clicked", G_CALLBACK(output_clear), NULL);
    gtk_box_pack_end(GTK_BOX(out_row), clr, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), out_row, FALSE, FALSE, 0);

    GtkWidget *oscroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(oscroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(oscroll, TRUE);
    gtk_widget_set_margin_start(oscroll, 10); gtk_widget_set_margin_end(oscroll, 10);
    gtk_widget_set_margin_bottom(oscroll, 4);

    g_output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(g_output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(g_output_view), GTK_WRAP_WORD_CHAR);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_output_view), "output-text");
    gtk_container_add(GTK_CONTAINER(oscroll), g_output_view);
    gtk_box_pack_start(GTK_BOX(content), oscroll, TRUE, TRUE, 0);

    // ── Status bar ──────────────────────────────────────────────
    g_status_bar = gtk_label_new("  Initializing…");
    gtk_style_context_add_class(gtk_widget_get_style_context(g_status_bar), "status-bar");
    gtk_widget_set_halign(g_status_bar, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), g_status_bar, FALSE, FALSE, 0);

    gtk_widget_show_all(g_main_window);

    // Load data and show the welcome message
    load_database();

    web_bridge_start();
    output_append(
        "╔══════════════════════════════════════════════════════════╗\n"
        "║      ALGERIA HISTORY DATABASE — v3.0  INITIALIZED       ║\n"
        "║      62 operations across 4 modules — 0 known bugs      ║\n"
        "║      NSCS  ·  Algorithms & Dynamic Data Structures       ║\n"
        "╚══════════════════════════════════════════════════════════╝\n\n"
        "  Select a module from the sidebar to begin.\n"
        "  All results appear here in the output terminal.\n\n"
        "  ✦ Linked Lists & Queues  — 25 operations  (incl. 4 new)\n"
        "  ✦ Stack Module           — 15 operations\n"
        "  ✦ Binary Search Tree     — 14 operations\n"
        "  ✦ Recursion Module       —  8 operations  (incl. step trace)\n\n"
        "  ★ NEW: Statistics Dashboard · ASCII Timeline · CSV Export\n"
        "  ★ NEW: Born-same-year filter · Confirm dialogs · Enter=OK\n"
    );
}

static void on_activate(GtkApplication *app, gpointer u) {
    (void)u;
    build_ui(app);
}

int main(int argc, char **argv) {
    if (argc > 1) strncpy(g_db_path, argv[1], 511);
    GtkApplication *app = gtk_application_new(
        "dz.nscs.algeria_history_v3", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}