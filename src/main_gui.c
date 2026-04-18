#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// ═══════════════════════════════════════════════════════════════
//  DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════

typedef struct { int day, month, year; } Date;

typedef struct Node {
    char name[120];
    char definition[400];
    Date dob, dod;
    struct Node *next;
} TList;

typedef struct SNode {
    char name[120];
    char definition[400];
    Date dob, dod;
    struct SNode *next;
} SNode;

typedef struct { SNode *top; } TStack;

typedef struct QNode {
    char name[120];
    char definition[400];
    Date dob, dod;
    struct QNode *next;
} QNode;

typedef struct { QNode *head, *tail; } TQueue;

typedef struct TTree {
    char name[120];
    char definition[400];
    Date dob, dod;
    struct TTree *left, *right;
} TTree;

// ═══════════════════════════════════════════════════════════════
//  GLOBAL STATE
// ═══════════════════════════════════════════════════════════════

static TList  *g_personalities = NULL;
static TList  *g_events        = NULL;
static TStack *g_stack         = NULL;
static TTree  *g_tree          = NULL;
static char    g_db_path[512]  = "database.txt";

// UI references
static GtkWidget *g_main_window;
static GtkWidget *g_stack_pages;   // GtkStack for module pages
static GtkWidget *g_output_view;   // shared GtkTextView
static GtkWidget *g_status_bar;
static GtkWidget *g_search_entry;

// ═══════════════════════════════════════════════════════════════
//  CSS THEME  — Algerian Desert + Ottoman Manuscript aesthetic
// ═══════════════════════════════════════════════════════════════

static const char *ALGERIA_CSS =
"@import url('https://fonts.googleapis.com/css2?family=Cinzel:wght@400;700&display=swap');"

/* Root palette */
"* { -gtk-icon-style: regular; }"

"window {"
"  background-color: #1a0f0a;"
"}"

/* Sidebar */
".sidebar {"
"  background-color: #0f0905;"
"  border-right: 2px solid #8B6914;"
"  min-width: 220px;"
"}"

".sidebar-title {"
"  font-size: 11px;"
"  font-weight: bold;"
"  color: #c8960c;"
"  letter-spacing: 3px;"
"  padding: 6px 16px 4px 16px;"
"}"

".nav-btn {"
"  background: transparent;"
"  border: none;"
"  border-radius: 0;"
"  color: #d4a853;"
"  font-size: 13px;"
"  padding: 10px 20px;"
"  text-align: left;"
"  border-left: 3px solid transparent;"
"  transition: all 200ms ease;"
"}"
".nav-btn:hover {"
"  background-color: rgba(200,150,12,0.12);"
"  border-left-color: #c8960c;"
"  color: #f0c040;"
"}"
".nav-btn.active {"
"  background-color: rgba(200,150,12,0.18);"
"  border-left-color: #e8a000;"
"  color: #ffe090;"
"  font-weight: bold;"
"}"

/* Header bar */
".app-header {"
"  background: linear-gradient(90deg, #0f0905 0%, #1e1208 50%, #0f0905 100%);"
"  border-bottom: 1px solid #8B6914;"
"  padding: 0 20px;"
"  min-height: 56px;"
"}"

".app-title {"
"  font-size: 20px;"
"  font-weight: bold;"
"  color: #f0c040;"
"  letter-spacing: 2px;"
"}"

".app-subtitle {"
"  font-size: 10px;"
"  color: #a07828;"
"  letter-spacing: 4px;"
"}"

/* Module content area */
".content-area {"
"  background-color: #120b06;"
"  padding: 24px;"
"}"

".module-title {"
"  font-size: 22px;"
"  font-weight: bold;"
"  color: #e8a000;"
"  letter-spacing: 1px;"
"  margin-bottom: 4px;"
"}"

".module-subtitle {"
"  font-size: 11px;"
"  color: #806030;"
"  letter-spacing: 3px;"
"  margin-bottom: 20px;"
"}"

/* Cards */
".func-card {"
"  background-color: #1e1208;"
"  border: 1px solid #3a2810;"
"  border-radius: 6px;"
"  padding: 14px 16px;"
"  margin-bottom: 10px;"
"}"

".func-card:hover {"
"  border-color: #8B6914;"
"  background-color: #231508;"
"}"

".func-name {"
"  font-size: 13px;"
"  font-weight: bold;"
"  color: #d4a853;"
"  font-family: monospace;"
"}"

".func-desc {"
"  font-size: 11px;"
"  color: #806030;"
"  margin-top: 3px;"
"}"

/* Action buttons */
".action-btn {"
"  background-color: #2a1a08;"
"  border: 1px solid #8B6914;"
"  border-radius: 4px;"
"  color: #f0c040;"
"  font-size: 12px;"
"  font-weight: bold;"
"  padding: 7px 16px;"
"  letter-spacing: 1px;"
"  transition: all 150ms;"
"}"
".action-btn:hover {"
"  background-color: #8B6914;"
"  color: #fffbe0;"
"}"
".action-btn:active {"
"  background-color: #6a4a0a;"
"}"

".danger-btn {"
"  border-color: #8b2020;"
"  color: #e05050;"
"}"
".danger-btn:hover {"
"  background-color: #8b2020;"
"  color: #ffe0e0;"
"}"

/* Output terminal */
".output-frame {"
"  background-color: #080503;"
"  border: 1px solid #2a1a08;"
"  border-radius: 4px;"
"}"

"textview.output-text {"
"  background-color: #080503;"
"  color: #c8a050;"
"  font-family: monospace;"
"  font-size: 12px;"
"  padding: 12px;"
"}"

"textview.output-text text {"
"  background-color: #080503;"
"  color: #c8a050;"
"}"

/* Inputs */
"entry {"
"  background-color: #1a0f0a;"
"  border: 1px solid #3a2810;"
"  border-radius: 4px;"
"  color: #e8c870;"
"  font-size: 13px;"
"  padding: 6px 10px;"
"  caret-color: #e8a000;"
"}"
"entry:focus {"
"  border-color: #8B6914;"
"  box-shadow: 0 0 0 2px rgba(139,105,20,0.25);"
"}"

/* Status bar */
".status-bar {"
"  background-color: #0f0905;"
"  border-top: 1px solid #2a1a08;"
"  color: #806030;"
"  font-size: 11px;"
"  padding: 4px 16px;"
"}"

/* Separator ornament */
".gold-sep {"
"  background-color: #8B6914;"
"  min-height: 1px;"
"  margin: 12px 0;"
"}"

/* Badge */
".badge {"
"  background-color: #8B6914;"
"  border-radius: 10px;"
"  color: #fffbe0;"
"  font-size: 10px;"
"  font-weight: bold;"
"  padding: 2px 8px;"
"}"

/* Scrollbars */
"scrollbar {"
"  background-color: #0f0905;"
"  border: none;"
"  min-width: 8px;"
"  min-height: 8px;"
"}"
"scrollbar slider {"
"  background-color: #3a2810;"
"  border-radius: 4px;"
"  min-width: 6px;"
"  min-height: 6px;"
"}"
"scrollbar slider:hover {"
"  background-color: #8B6914;"
"}"

/* Dialog */
"dialog {"
"  background-color: #1a0f0a;"
"}"
".dialog-title {"
"  font-size: 16px;"
"  font-weight: bold;"
"  color: #e8a000;"
"  padding: 16px 20px 8px;"
"}"
;

// ═══════════════════════════════════════════════════════════════
//  HELPER UTILITIES
// ═══════════════════════════════════════════════════════════════

static void output_clear(void) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_output_view));
    gtk_text_buffer_set_text(buf, "", -1);
}

static void output_append(const char *text) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_output_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_insert(buf, &end, text, -1);
    // scroll to end
    GtkTextMark *mark = gtk_text_buffer_get_insert(buf);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(g_output_view), mark);
}

static void output_appendf(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    output_append(buf);
}

static void set_status(const char *msg) {
    gtk_label_set_text(GTK_LABEL(g_status_bar), msg);
}

static Date parse_date(const char *s) {
    Date d = {1,1,0};
    if (!s || !*s) return d;
    // try YYYY only
    if (strlen(s) <= 4) { d.year = atoi(s); d.day=1; d.month=1; return d; }
    sscanf(s, "%d/%d/%d", &d.day, &d.month, &d.year);
    return d;
}

static void format_date(char *out, Date d) {
    if (d.day==1 && d.month==1)
        snprintf(out, 20, "%d", d.year);
    else
        snprintf(out, 20, "%02d/%02d/%04d", d.day, d.month, d.year);
}

// ═══════════════════════════════════════════════════════════════
//  DATABASE LOAD
// ═══════════════════════════════════════════════════════════════

static TList* new_node(const char *name, const char *def, Date dob, Date dod) {
    TList *n = calloc(1, sizeof(TList));
    strncpy(n->name,       name ? name : "", 119);
    strncpy(n->definition, def  ? def  : "", 399);
    n->dob = dob; n->dod = dod;
    return n;
}

static void free_list(TList *head) {
    while (head) { TList *t=head->next; free(head); head=t; }
}

static void load_database(void) {
    free_list(g_personalities);
    free_list(g_events);
    g_personalities = NULL;
    g_events        = NULL;

    FILE *f = fopen(g_db_path, "r");
    if (!f) {
        set_status("  Database file not found — using empty dataset");
        return;
    }
    char line[600];
    TList *ptail=NULL, *etail=NULL;
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line,"\r\n")] = 0;
        if (!line[0]) continue;

        if (strchr(line,'=')) {
            // personality: name=def=year=year
            char *p1=strtok(line,"="), *p2=strtok(NULL,"="),
                 *p3=strtok(NULL,"="), *p4=strtok(NULL,"=");
            if (!p1||!p2) continue;
            Date dob=parse_date(p3), dod=parse_date(p4);
            TList *node = new_node(p1,p2,dob,dod);
            if (!g_personalities) g_personalities=ptail=node;
            else { ptail->next=node; ptail=node; }
        } else if (strchr(line,':')) {
            // event: name:def{year}
            char name[200]={0}, def[400]={0}, yearstr[20]={0};
            char *col=strchr(line,':');
            strncpy(name,line,col-line);
            char *rest=col+1;
            char *ob=strchr(rest,'{'), *cb=strchr(rest,'}');
            if (ob&&cb) {
                strncpy(def,rest,ob-rest);
                strncpy(yearstr,ob+1,cb-ob-1);
            } else strncpy(def,rest,399);
            Date dob=parse_date(yearstr), dod={0,0,0};
            TList *node = new_node(name,def,dob,dod);
            if (!g_events) g_events=etail=node;
            else { etail->next=node; etail=node; }
        }
    }
    fclose(f);
    char msg[128];
    int pc=0,ec=0;
    for(TList*n=g_personalities;n;n=n->next)pc++;
    for(TList*n=g_events;n;n=n->next)ec++;
    snprintf(msg,sizeof(msg),"  Database loaded — %d personalities, %d events",pc,ec);
    set_status(msg);
}

static void save_database(void) {
    FILE *f = fopen(g_db_path,"w");
    if (!f) { set_status("  ERROR: Cannot write database file"); return; }
    for (TList *n=g_personalities;n;n=n->next) {
        char ds1[20],ds2[20];
        format_date(ds1,n->dob); format_date(ds2,n->dod);
        fprintf(f,"%s=%s=%s=%s\n",n->name,n->definition,ds1,ds2);
    }
    for (TList *n=g_events;n;n=n->next) {
        char ds1[20];
        format_date(ds1,n->dob);
        fprintf(f,"%s:%s{%s}\n",n->name,n->definition,ds1);
    }
    fclose(f);
    set_status("  Database saved successfully");
}

// ═══════════════════════════════════════════════════════════════
//  LINKED LIST MODULE FUNCTIONS
// ═══════════════════════════════════════════════════════════════

static void ll_list_all(void) {
    output_clear();
    output_append("╔══════════════════════════════════════════════════════════╗\n");
    output_append("║           PERSONALITIES OF ALGERIAN HISTORY              ║\n");
    output_append("╚══════════════════════════════════════════════════════════╝\n\n");
    if (!g_personalities) { output_append("  (no personalities loaded)\n"); return; }
    int i=1;
    for (TList *n=g_personalities;n;n=n->next,i++) {
        char ds1[20],ds2[20];
        format_date(ds1,n->dob); format_date(ds2,n->dod);
        output_appendf("  [%d] %s\n", i, n->name);
        output_appendf("      %s\n", n->definition);
        output_appendf("      Born: %s  |  Died: %s\n\n", ds1, ds2);
    }
}

static void ll_list_events(void) {
    output_clear();
    output_append("╔══════════════════════════════════════════════════════════╗\n");
    output_append("║              HISTORICAL EVENTS OF ALGERIA                ║\n");
    output_append("╚══════════════════════════════════════════════════════════╝\n\n");
    if (!g_events) { output_append("  (no events loaded)\n"); return; }
    int i=1;
    for (TList *n=g_events;n;n=n->next,i++) {
        char ds1[20];
        format_date(ds1,n->dob);
        output_appendf("  [%d] %s  (%s)\n", i, n->name, ds1);
        output_appendf("      %s\n\n", n->definition);
    }
}

static void ll_sort_alpha(void) {
    // selection sort
    for (TList *i=g_personalities;i;i=i->next) {
        TList *mn=i;
        for (TList *j=i->next;j;j=j->next)
            if (strcmp(j->name,mn->name)<0) mn=j;
        if (mn!=i) {
            char tmp[120]; strncpy(tmp,i->name,119);
            strncpy(i->name,mn->name,119); strncpy(mn->name,tmp,119);
            char td[400]; strncpy(td,i->definition,399);
            strncpy(i->definition,mn->definition,399); strncpy(mn->definition,td,399);
            Date dt=i->dob; i->dob=mn->dob; mn->dob=dt;
            dt=i->dod; i->dod=mn->dod; mn->dod=dt;
        }
    }
    output_clear();
    output_append("✦ Personalities sorted alphabetically:\n\n");
    ll_list_all();
}

static void ll_search_by_name(const char *name) {
    output_clear();
    output_appendf("╔══════════ SEARCH: \"%s\" ══════════╗\n\n", name);
    bool found=false;
    for (TList *n=g_personalities;n;n=n->next) {
        if (strstr(n->name,name)||strstr(n->definition,name)) {
            char ds1[20],ds2[20];
            format_date(ds1,n->dob); format_date(ds2,n->dod);
            output_appendf("  ► %s\n", n->name);
            output_appendf("    %s\n", n->definition);
            output_appendf("    Born: %s  |  Died: %s\n\n", ds1, ds2);
            found=true;
        }
    }
    for (TList *n=g_events;n;n=n->next) {
        if (strstr(n->name,name)||strstr(n->definition,name)) {
            char ds1[20]; format_date(ds1,n->dob);
            output_appendf("  ► [EVENT] %s (%s)\n", n->name, ds1);
            output_appendf("    %s\n\n", n->definition);
            found=true;
        }
    }
    if (!found) output_append("  No results found.\n");
}

// Add personality
static void ll_add_personality(const char *name,const char *def,const char *dob,const char *dod) {
    TList *node = new_node(name,def,parse_date(dob),parse_date(dod));
    if (!g_personalities) { g_personalities=node; }
    else { TList *t=g_personalities; while(t->next)t=t->next; t->next=node; }
    save_database();
    output_clear();
    output_appendf("✦ Added personality: %s\n", name);
    ll_list_all();
}

// Delete personality
static void ll_delete_personality(const char *name) {
    TList *prev=NULL, *cur=g_personalities;
    while (cur) {
        if (strcasecmp(cur->name,name)==0) {
            if (prev) prev->next=cur->next; else g_personalities=cur->next;
            free(cur);
            save_database();
            output_clear();
            output_appendf("✦ Deleted: %s\n\n", name);
            ll_list_all();
            return;
        }
        prev=cur; cur=cur->next;
    }
    output_appendf("  Not found: %s\n", name);
}

// Sort by age (year of death - year of birth)
static void ll_sort_by_age(void) {
    for (TList *i=g_personalities;i;i=i->next) {
        TList *mn=i;
        int age_mn = mn->dod.year - mn->dob.year;
        for (TList *j=i->next;j;j=j->next) {
            int age_j = j->dod.year - j->dob.year;
            if (age_j < age_mn) { mn=j; age_mn=age_j; }
        }
        if (mn!=i) {
            char tmp[120]; strncpy(tmp,i->name,119);
            strncpy(i->name,mn->name,119); strncpy(mn->name,tmp,119);
            char td[400]; strncpy(td,i->definition,399);
            strncpy(i->definition,mn->definition,399); strncpy(mn->definition,td,399);
            Date dt=i->dob; i->dob=mn->dob; mn->dob=dt;
            dt=i->dod; i->dod=mn->dod; mn->dod=dt;
        }
    }
    output_clear();
    output_append("✦ Sorted by age (ascending):\n\n");
    for (TList *n=g_personalities;n;n=n->next) {
        int age=n->dod.year-n->dob.year;
        output_appendf("  %s  (age: %d)\n",n->name,age);
    }
}

// ═══════════════════════════════════════════════════════════════
//  STACK MODULE
// ═══════════════════════════════════════════════════════════════

static void stk_push(TStack *s,const char*name,const char*def,Date dob,Date dod){
    SNode *n=calloc(1,sizeof(SNode));
    strncpy(n->name,name,119); strncpy(n->definition,def,399);
    n->dob=dob; n->dod=dod; n->next=s->top; s->top=n;
}
static SNode* stk_pop(TStack *s){
    if(!s->top)return NULL;
    SNode*t=s->top; s->top=s->top->next; t->next=NULL; return t;
}
static bool stk_empty(TStack *s){ return s->top==NULL; }

static void stk_build_from_list(void) {
    if (!g_stack) g_stack=calloc(1,sizeof(TStack));
    // clear
    while(!stk_empty(g_stack)) { SNode*t=stk_pop(g_stack); free(t); }
    for (TList *n=g_personalities;n;n=n->next)
        stk_push(g_stack,n->name,n->definition,n->dob,n->dod);
    output_clear();
    output_append("✦ Stack built from personality list:\n\n");
    for (SNode *n=g_stack->top;n;n=n->next)
        output_appendf("  [TOP→] %s\n",n->name);
}

static void stk_display(void) {
    output_clear();
    output_append("╔══════════════════════════════════════════════════════════╗\n");
    output_append("║                    STACK CONTENTS                       ║\n");
    output_append("╚══════════════════════════════════════════════════════════╝\n\n");
    if (!g_stack||stk_empty(g_stack)){output_append("  Stack is empty.\n");return;}
    int i=1;
    for(SNode*n=g_stack->top;n;n=n->next,i++){
        char ds1[20],ds2[20]; format_date(ds1,n->dob); format_date(ds2,n->dod);
        output_appendf("  [%d] %s\n      %s  |  %s–%s\n\n",i,n->name,n->definition,ds1,ds2);
    }
}

static void stk_sort_alpha(void) {
    if(!g_stack||stk_empty(g_stack)){output_append("  Stack empty.\n");return;}
    // collect into array then sort
    int cnt=0; for(SNode*n=g_stack->top;n;n=n->next)cnt++;
    SNode **arr=malloc(cnt*sizeof(SNode*));
    int k=0; for(SNode*n=g_stack->top;n;n=n->next)arr[k++]=n;
    for(int i=0;i<cnt;i++) for(int j=i+1;j<cnt;j++)
        if(strcmp(arr[i]->name,arr[j]->name)>0){ SNode*t=arr[i];arr[i]=arr[j];arr[j]=t; }
    // relink
    for(int i=0;i<cnt-1;i++) arr[i]->next=arr[i+1];
    arr[cnt-1]->next=NULL; g_stack->top=arr[0];
    free(arr);
    output_clear();
    output_append("✦ Stack sorted alphabetically:\n\n");
    stk_display();
}

static void stk_search(const char *name){
    output_clear();
    if(!g_stack){output_append("  Stack not built.\n");return;}
    for(SNode*n=g_stack->top;n;n=n->next){
        if(strcasecmp(n->name,name)==0){
            char ds1[20],ds2[20]; format_date(ds1,n->dob); format_date(ds2,n->dod);
            output_appendf("✦ Found in stack:\n\n  %s\n  %s\n  Born: %s  Died: %s\n",
                n->name,n->definition,ds1,ds2);
            return;
        }
    }
    output_appendf("  '%s' not found in stack.\n",name);
}

static void stk_reverse(void) {
    if(!g_stack||stk_empty(g_stack)){output_append("  Stack empty.\n");return;}
    TStack tmp={NULL};
    while(!stk_empty(g_stack)){
        SNode*n=stk_pop(g_stack);
        stk_push(&tmp,n->name,n->definition,n->dob,n->dod);
        free(n);
    }
    g_stack->top=tmp.top;
    output_clear();
    output_append("✦ Stack reversed:\n\n");
    stk_display();
}

// ═══════════════════════════════════════════════════════════════
//  BST MODULE
// ═══════════════════════════════════════════════════════════════

static TTree* bst_insert(TTree *root,const char*name,const char*def,Date dob,Date dod){
    if(!root){
        TTree*n=calloc(1,sizeof(TTree));
        strncpy(n->name,name,119); strncpy(n->definition,def,399);
        n->dob=dob; n->dod=dod; return n;
    }
    int c=strcmp(name,root->name);
    if(c<0) root->left=bst_insert(root->left,name,def,dob,dod);
    else if(c>0) root->right=bst_insert(root->right,name,def,dob,dod);
    return root;
}

static TTree* bst_find_min(TTree *n){
    while(n->left) n=n->left; return n;
}

static TTree* bst_delete(TTree *root, const char *name){
    if(!root) return NULL;
    int c=strcmp(name,root->name);
    if(c<0) root->left=bst_delete(root->left,name);
    else if(c>0) root->right=bst_delete(root->right,name);
    else {
        if(!root->left){ TTree*t=root->right; free(root); return t; }
        if(!root->right){ TTree*t=root->left; free(root); return t; }
        TTree*mn=bst_find_min(root->right);
        strncpy(root->name,mn->name,119);
        strncpy(root->definition,mn->definition,399);
        root->dob=mn->dob; root->dod=mn->dod;
        root->right=bst_delete(root->right,mn->name);
    }
    return root;
}

static void bst_inorder_print(TTree *n){
    if(!n) return;
    bst_inorder_print(n->left);
    char ds1[20],ds2[20]; format_date(ds1,n->dob); format_date(ds2,n->dod);
    output_appendf("  ► %s  (%s – %s)\n",n->name,ds1,ds2);
    bst_inorder_print(n->right);
}
static void bst_preorder_print(TTree *n){
    if(!n) return;
    char ds1[20],ds2[20]; format_date(ds1,n->dob); format_date(ds2,n->dod);
    output_appendf("  ► %s  (%s – %s)\n",n->name,ds1,ds2);
    bst_preorder_print(n->left);
    bst_preorder_print(n->right);
}
static void bst_postorder_print(TTree *n){
    if(!n) return;
    bst_postorder_print(n->left);
    bst_postorder_print(n->right);
    char ds1[20],ds2[20]; format_date(ds1,n->dob); format_date(ds2,n->dod);
    output_appendf("  ► %s  (%s – %s)\n",n->name,ds1,ds2);
}

static int bst_height(TTree *n){
    if(!n) return 0;
    int l=bst_height(n->left), r=bst_height(n->right);
    return 1+(l>r?l:r);
}
static int bst_size(TTree *n){ return n?1+bst_size(n->left)+bst_size(n->right):0; }

static void bst_build(void){
    // free old
    // simple free
    g_tree=NULL;
    for(TList*n=g_personalities;n;n=n->next)
        g_tree=bst_insert(g_tree,n->name,n->definition,n->dob,n->dod);
    output_clear();
    output_appendf("✦ BST built — %d nodes, height %d\n\n",bst_size(g_tree),bst_height(g_tree));
    output_append("  In-order traversal:\n");
    bst_inorder_print(g_tree);
}

static TTree* bst_search_node(TTree *root,const char*name){
    if(!root) return NULL;
    int c=strcmp(name,root->name);
    if(c==0) return root;
    if(c<0) return bst_search_node(root->left,name);
    return bst_search_node(root->right,name);
}

// ═══════════════════════════════════════════════════════════════
//  RECURSION MODULE
// ═══════════════════════════════════════════════════════════════

static int rec_count_occ(TList *f, const char *name){
    if(!f) return 0;
    return (strcasecmp(f->name,name)==0?1:0)+rec_count_occ(f->next,name);
}

static bool rec_is_palindrome(const char *s, int l, int r){
    if(l>=r) return true;
    if(s[l]!=s[r]) return false;
    return rec_is_palindrome(s,l+1,r-1);
}

static int rec_distinct_subseq(const char *s){
    if(!s||!*s) return 1;
    int cnt=2*rec_distinct_subseq(s+1);
    for(int i=1;s[i];i++) if(s[i]==s[0]){ cnt-=rec_distinct_subseq(s+i+1); break; }
    return cnt;
}

static void rec_permutations_helper(char *s, int start, int end, int *count){
    if(start==end){ (*count)++; if(*count<=20) output_appendf("  %s\n",s); return; }
    for(int i=start;i<=end;i++){
        char t=s[start]; s[start]=s[i]; s[i]=t;
        rec_permutations_helper(s,start+1,end,count);
        t=s[start]; s[start]=s[i]; s[i]=t;
    }
}

// ═══════════════════════════════════════════════════════════════
//  DIALOG HELPERS
// ═══════════════════════════════════════════════════════════════

static GtkWidget* make_entry(GtkWidget *grid, const char *label, int row) {
    GtkWidget *lbl = gtk_label_new(label);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_widget_set_margin_bottom(lbl,2);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl),"func-desc");
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, row*2,   2, 1);
    GtkWidget *ent = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), ent, 0, row*2+1, 2, 1);
    gtk_widget_set_margin_bottom(ent,10);
    return ent;
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — Linked Lists
// ═══════════════════════════════════════════════════════════════

static void on_ll_list_personalities(GtkButton *b, gpointer u){ ll_list_all(); }
static void on_ll_list_events(GtkButton *b, gpointer u){ ll_list_events(); }
static void on_ll_sort_alpha(GtkButton *b, gpointer u){ ll_sort_alpha(); }
static void on_ll_sort_age(GtkButton *b, gpointer u){ ll_sort_by_age(); }
static void on_ll_reload(GtkButton *b, gpointer u){ load_database(); ll_list_all(); }

static void on_ll_search(GtkButton *b, gpointer u){
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
        "Search", GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
        "Search", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *ca = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *lbl=gtk_label_new("Search name or keyword:");
    gtk_widget_set_halign(lbl,GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(ca),lbl);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK)
        ll_search_by_name(gtk_entry_get_text(GTK_ENTRY(ent)));
    gtk_widget_destroy(dlg);
}

static void on_ll_add(GtkButton *b, gpointer u){
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
        "Add Personality", GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
        "Add", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *ca = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *grid=gtk_grid_new(); gtk_grid_set_row_spacing(GTK_GRID(grid),2);
    GtkWidget *e_name=make_entry(grid,"Full Name",0);
    GtkWidget *e_def =make_entry(grid,"Definition",1);
    GtkWidget *e_dob =make_entry(grid,"Year of Birth (YYYY)",2);
    GtkWidget *e_dod =make_entry(grid,"Year of Death (YYYY)",3);
    gtk_container_add(GTK_CONTAINER(ca),grid);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK)
        ll_add_personality(gtk_entry_get_text(GTK_ENTRY(e_name)),
            gtk_entry_get_text(GTK_ENTRY(e_def)),
            gtk_entry_get_text(GTK_ENTRY(e_dob)),
            gtk_entry_get_text(GTK_ENTRY(e_dod)));
    gtk_widget_destroy(dlg);
}

static void on_ll_delete(GtkButton *b, gpointer u){
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
        "Delete Personality", GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
        "Delete", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *ca = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *lbl=gtk_label_new("Enter exact name to delete:");
    gtk_widget_set_halign(lbl,GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(ca),lbl);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK)
        ll_delete_personality(gtk_entry_get_text(GTK_ENTRY(ent)));
    gtk_widget_destroy(dlg);
}

static void on_ll_add_event(GtkButton *b, gpointer u){
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
        "Add Event", GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
        "Add", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *ca = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *grid=gtk_grid_new();
    GtkWidget *e_name=make_entry(grid,"Event Name",0);
    GtkWidget *e_def =make_entry(grid,"Description",1);
    GtkWidget *e_year=make_entry(grid,"Year",2);
    gtk_container_add(GTK_CONTAINER(ca),grid);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        TList *node=new_node(
            gtk_entry_get_text(GTK_ENTRY(e_name)),
            gtk_entry_get_text(GTK_ENTRY(e_def)),
            parse_date(gtk_entry_get_text(GTK_ENTRY(e_year))),
            (Date){0,0,0});
        if(!g_events) g_events=node;
        else { TList*t=g_events; while(t->next)t=t->next; t->next=node; }
        save_database();
        ll_list_events();
    }
    gtk_widget_destroy(dlg);
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — Stack
// ═══════════════════════════════════════════════════════════════

static void on_stk_build(GtkButton *b,gpointer u){ stk_build_from_list(); }
static void on_stk_display(GtkButton *b,gpointer u){ stk_display(); }
static void on_stk_sort(GtkButton *b,gpointer u){ stk_sort_alpha(); }
static void on_stk_reverse(GtkButton *b,gpointer u){ stk_reverse(); }

static void on_stk_search(GtkButton *b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Search Stack",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Search",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK)
        stk_search(gtk_entry_get_text(GTK_ENTRY(ent)));
    gtk_widget_destroy(dlg);
}

static void on_stk_delete(GtkButton *b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Delete from Stack",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Delete",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char *name=gtk_entry_get_text(GTK_ENTRY(ent));
        if(!g_stack){output_append("Stack not built.\n");goto done_del;}
        SNode *prev=NULL,*cur=g_stack->top;
        while(cur){ if(strcasecmp(cur->name,name)==0){
            if(prev)prev->next=cur->next; else g_stack->top=cur->next;
            free(cur); output_clear();
            output_appendf("✦ Deleted '%s' from stack.\n\n",name);
            stk_display(); goto done_del;
        } prev=cur; cur=cur->next; }
        output_appendf("  '%s' not found.\n",name);
        done_del:;
    }
    gtk_widget_destroy(dlg);
}

static void on_stk_get_smallest(GtkButton *b,gpointer u){
    output_clear();
    if(!g_stack||stk_empty(g_stack)){output_append("Stack empty.\n");return;}
    SNode *smallest=g_stack->top;
    int mc=0; for(char*p=smallest->definition;*p;p++) if(*p==' ')mc++;
    for(SNode*n=g_stack->top->next;n;n=n->next){
        int wc=0; for(char*p=n->definition;*p;p++) if(*p==' ')wc++;
        if(wc<mc){mc=wc;smallest=n;}
    }
    output_appendf("✦ Shortest definition (%d words):\n\n  %s\n  \"%s\"\n",
        mc+1,smallest->name,smallest->definition);
}

static void on_stk_overlapping(GtkButton *b,gpointer u){
    output_clear();
    output_append("╔══════════════════════════════════════════════════════════╗\n");
    output_append("║               OVERLAPPING DATE RANGES                   ║\n");
    output_append("╚══════════════════════════════════════════════════════════╝\n\n");
    if(!g_stack){output_append("  Build stack first.\n");return;}
    bool any=false;
    for(SNode*a=g_stack->top;a;a=a->next)
    for(SNode*b2=a->next;b2;b2=b2->next){
        int s1=a->dob.year,e1=a->dod.year,s2=b2->dob.year,e2=b2->dod.year;
        if(s1<=e2&&s2<=e1){
            output_appendf("  ► %s  (%d–%d)\n    %s  (%d–%d)\n\n",
                a->name,s1,e1,b2->name,s2,e2);
            any=true;
        }
    }
    if(!any) output_append("  No overlapping date ranges found.\n");
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — BST
// ═══════════════════════════════════════════════════════════════

static void on_bst_build(GtkButton*b,gpointer u){ bst_build(); }
static void on_bst_inorder(GtkButton*b,gpointer u){
    output_clear(); output_append("✦ In-order traversal (sorted):\n\n");
    if(!g_tree){output_append("  Build BST first.\n");return;}
    bst_inorder_print(g_tree);
}
static void on_bst_preorder(GtkButton*b,gpointer u){
    output_clear(); output_append("✦ Pre-order traversal:\n\n");
    if(!g_tree){output_append("  Build BST first.\n");return;}
    bst_preorder_print(g_tree);
}
static void on_bst_postorder(GtkButton*b,gpointer u){
    output_clear(); output_append("✦ Post-order traversal:\n\n");
    if(!g_tree){output_append("  Build BST first.\n");return;}
    bst_postorder_print(g_tree);
}
static void on_bst_stats(GtkButton*b,gpointer u){
    output_clear();
    if(!g_tree){output_append("  Build BST first.\n");return;}
    output_appendf("✦ BST Statistics:\n\n  Nodes  : %d\n  Height : %d\n",
        bst_size(g_tree),bst_height(g_tree));
}
static void on_bst_search(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Search BST",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Search",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        if(!g_tree){output_append("Build BST first.\n");}
        else {
            TTree *found=bst_search_node(g_tree,gtk_entry_get_text(GTK_ENTRY(ent)));
            output_clear();
            if(found){
                char ds1[20],ds2[20]; format_date(ds1,found->dob); format_date(ds2,found->dod);
                output_appendf("✦ Found in BST:\n\n  %s\n  %s\n  Born: %s  Died: %s\n",
                    found->name,found->definition,ds1,ds2);
            } else output_append("  Not found in BST.\n");
        }
    }
    gtk_widget_destroy(dlg);
}
static void on_bst_delete(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Delete from BST",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Delete",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char*name=gtk_entry_get_text(GTK_ENTRY(ent));
        g_tree=bst_delete(g_tree,name);
        output_clear();
        output_appendf("✦ Deleted '%s' from BST.\n\n",name);
        output_appendf("  Remaining nodes: %d  |  Height: %d\n",bst_size(g_tree),bst_height(g_tree));
    }
    gtk_widget_destroy(dlg);
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLERS — Recursion
// ═══════════════════════════════════════════════════════════════

static void on_rec_count(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Count Occurrences",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Count",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char*name=gtk_entry_get_text(GTK_ENTRY(ent));
        int c=rec_count_occ(g_personalities,name);
        output_clear();
        output_appendf("✦ Occurrences of \"%s\":\n\n  Found %d time(s) in personalities list.\n",name,c);
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_palindrome(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Palindrome Check",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Check",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char*word=gtk_entry_get_text(GTK_ENTRY(ent));
        bool ok=rec_is_palindrome(word,0,(int)strlen(word)-1);
        output_clear();
        output_appendf("✦ Palindrome check: \"%s\"\n\n  Result: %s\n",
            word, ok?"YES — it is a palindrome!":"NO — not a palindrome.");
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_permutation(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Permutations",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Generate",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char*word=gtk_entry_get_text(GTK_ENTRY(ent));
        char buf[128]; strncpy(buf,word,127);
        output_clear();
        output_appendf("✦ Permutations of \"%s\" (max 20 shown):\n\n",word);
        int cnt=0;
        rec_permutations_helper(buf,0,(int)strlen(buf)-1,&cnt);
        output_appendf("\n  Total permutations: %d\n",cnt);
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_distinct(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Distinct Subsequences",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Count",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char*word=gtk_entry_get_text(GTK_ENTRY(ent));
        int c=rec_distinct_subseq(word);
        output_clear();
        output_appendf("✦ Distinct subsequences of \"%s\":\n\n  Count: %d\n",word,c);
    }
    gtk_widget_destroy(dlg);
}

static void on_rec_remove(GtkButton*b,gpointer u){
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Remove Occurrences",GTK_WINDOW(g_main_window),
        GTK_DIALOG_MODAL,"Remove",GTK_RESPONSE_OK,"Cancel",GTK_RESPONSE_CANCEL,NULL);
    GtkWidget *ca=gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(ca),16);
    GtkWidget *ent=gtk_entry_new(); gtk_container_add(GTK_CONTAINER(ca),ent);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char*name=gtk_entry_get_text(GTK_ENTRY(ent));
        // recursive remove
        TList *prev=NULL,*cur=g_personalities;
        while(cur){
            if(strcasecmp(cur->name,name)==0){
                TList*nx=cur->next;
                if(prev)prev->next=nx; else g_personalities=nx;
                free(cur); cur=nx;
            } else { prev=cur; cur=cur->next; }
        }
        save_database();
        output_clear();
        output_appendf("✦ Removed all occurrences of \"%s\" (recursive).\n\n",name);
        ll_list_all();
    }
    gtk_widget_destroy(dlg);
}

// ═══════════════════════════════════════════════════════════════
//  NAV BUTTON HANDLER
// ═══════════════════════════════════════════════════════════════

typedef struct { GtkWidget *btn; GList *all_btns; const char *page_name; } NavData;

static void on_nav_click(GtkButton *b, gpointer user_data){
    NavData *nd = (NavData*)user_data;
    // remove active from all
    for(GList*l=nd->all_btns;l;l=l->next){
        GtkWidget *w=(GtkWidget*)l->data;
        gtk_style_context_remove_class(gtk_widget_get_style_context(w),"active");
    }
    gtk_style_context_add_class(gtk_widget_get_style_context(nd->btn),"active");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack_pages), nd->page_name);
    output_clear();
}

// ═══════════════════════════════════════════════════════════════
//  PAGE BUILDERS
// ═══════════════════════════════════════════════════════════════

static GtkWidget* make_btn(const char *label, GCallback cb, gpointer data, bool danger){
    GtkWidget *b=gtk_button_new_with_label(label);
    gtk_style_context_add_class(gtk_widget_get_style_context(b),"action-btn");
    if(danger) gtk_style_context_add_class(gtk_widget_get_style_context(b),"danger-btn");
    if(cb) g_signal_connect(b,"clicked",cb,data);
    return b;
}

static GtkWidget* build_welcome_page(void){
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_valign(box,GTK_ALIGN_CENTER);
    gtk_widget_set_halign(box,GTK_ALIGN_CENTER);

    GtkWidget *title=gtk_label_new("الجزائر");
    gtk_style_context_add_class(gtk_widget_get_style_context(title),"module-title");
    gtk_label_set_justify(GTK_LABEL(title),GTK_JUSTIFY_CENTER);
    GtkWidget *sub=gtk_label_new("ALGERIA HISTORY DATABASE");
    gtk_style_context_add_class(gtk_widget_get_style_context(sub),"module-subtitle");
    GtkWidget *sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep),"gold-sep");
    gtk_widget_set_size_request(sep,300,-1);
    GtkWidget *desc=gtk_label_new(
        "A dynamic data structures project managing the personalities,\n"
        "events and dates of Algerian history.\n\n"
        "Navigate using the sidebar to explore each module.");
    gtk_label_set_justify(GTK_LABEL(desc),GTK_JUSTIFY_CENTER);
    gtk_style_context_add_class(gtk_widget_get_style_context(desc),"func-desc");

    gtk_box_pack_start(GTK_BOX(box),title,FALSE,FALSE,8);
    gtk_box_pack_start(GTK_BOX(box),sub,FALSE,FALSE,4);
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,16);
    gtk_box_pack_start(GTK_BOX(box),desc,FALSE,FALSE,8);
    return box;
}

static GtkWidget* build_ll_page(void){
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    GtkWidget *t=gtk_label_new("Linked Lists & Queues");
    gtk_style_context_add_class(gtk_widget_get_style_context(t),"module-title");
    gtk_widget_set_halign(t,GTK_ALIGN_START);
    GtkWidget *s=gtk_label_new("MODULE I  ·  ITERATIVE DATA MANAGEMENT");
    gtk_style_context_add_class(gtk_widget_get_style_context(s),"module-subtitle");
    gtk_widget_set_halign(s,GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box),t,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),s,FALSE,FALSE,0);

    GtkWidget *sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep),"gold-sep");
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,4);

    struct { const char *label; GCallback cb; bool danger; } btns[] = {
        {"List All Personalities",  G_CALLBACK(on_ll_list_personalities), false},
        {"List All Events",         G_CALLBACK(on_ll_list_events),        false},
        {"Search / Get Info",       G_CALLBACK(on_ll_search),             false},
        {"Add Personality",         G_CALLBACK(on_ll_add),                false},
        {"Add Event",               G_CALLBACK(on_ll_add_event),          false},
        {"Delete Personality",      G_CALLBACK(on_ll_delete),             true},
        {"Sort Alphabetically",     G_CALLBACK(on_ll_sort_alpha),         false},
        {"Sort by Age",             G_CALLBACK(on_ll_sort_age),           false},
        {"Reload Database",         G_CALLBACK(on_ll_reload),             false},
    };
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_grid_set_row_spacing(GTK_GRID(grid),10);
    int n=sizeof(btns)/sizeof(btns[0]);
    for(int i=0;i<n;i++){
        GtkWidget *b=make_btn(btns[i].label,btns[i].cb,NULL,btns[i].danger);
        gtk_widget_set_hexpand(b,TRUE);
        gtk_grid_attach(GTK_GRID(grid),b,i%3,i/3,1,1);
    }
    gtk_box_pack_start(GTK_BOX(box),grid,FALSE,FALSE,0);
    return box;
}

static GtkWidget* build_stack_page(void){
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    GtkWidget *t=gtk_label_new("Stack Operations");
    gtk_style_context_add_class(gtk_widget_get_style_context(t),"module-title");
    gtk_widget_set_halign(t,GTK_ALIGN_START);
    GtkWidget *s=gtk_label_new("MODULE II  ·  LIFO DATA STRUCTURE");
    gtk_style_context_add_class(gtk_widget_get_style_context(s),"module-subtitle");
    gtk_widget_set_halign(s,GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box),t,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),s,FALSE,FALSE,0);
    GtkWidget *sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep),"gold-sep");
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,4);

    struct { const char *label; GCallback cb; bool danger; } btns[] = {
        {"Build Stack from List",   G_CALLBACK(on_stk_build),        false},
        {"Display Stack",           G_CALLBACK(on_stk_display),      false},
        {"Search by Name",          G_CALLBACK(on_stk_search),       false},
        {"Sort Alphabetically",     G_CALLBACK(on_stk_sort),         false},
        {"Reverse Stack (Rec.)",    G_CALLBACK(on_stk_reverse),      false},
        {"Get Shortest Definition", G_CALLBACK(on_stk_get_smallest), false},
        {"Delete from Stack",       G_CALLBACK(on_stk_delete),       true},
        {"Show Overlapping Dates",  G_CALLBACK(on_stk_overlapping),  false},
    };
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_grid_set_row_spacing(GTK_GRID(grid),10);
    int n=sizeof(btns)/sizeof(btns[0]);
    for(int i=0;i<n;i++){
        GtkWidget *b=make_btn(btns[i].label,btns[i].cb,NULL,btns[i].danger);
        gtk_widget_set_hexpand(b,TRUE);
        gtk_grid_attach(GTK_GRID(grid),b,i%3,i/3,1,1);
    }
    gtk_box_pack_start(GTK_BOX(box),grid,FALSE,FALSE,0);
    return box;
}

static GtkWidget* build_bst_page(void){
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    GtkWidget *t=gtk_label_new("Binary Search Tree");
    gtk_style_context_add_class(gtk_widget_get_style_context(t),"module-title");
    gtk_widget_set_halign(t,GTK_ALIGN_START);
    GtkWidget *s=gtk_label_new("MODULE III  ·  HIERARCHICAL DATA STRUCTURE");
    gtk_style_context_add_class(gtk_widget_get_style_context(s),"module-subtitle");
    gtk_widget_set_halign(s,GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box),t,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),s,FALSE,FALSE,0);
    GtkWidget *sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep),"gold-sep");
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,4);

    struct { const char *label; GCallback cb; bool danger; } btns[] = {
        {"Build BST",              G_CALLBACK(on_bst_build),     false},
        {"In-Order Traversal",     G_CALLBACK(on_bst_inorder),   false},
        {"Pre-Order Traversal",    G_CALLBACK(on_bst_preorder),  false},
        {"Post-Order Traversal",   G_CALLBACK(on_bst_postorder), false},
        {"Search in BST",          G_CALLBACK(on_bst_search),    false},
        {"Delete from BST",        G_CALLBACK(on_bst_delete),    true},
        {"Height & Size Stats",    G_CALLBACK(on_bst_stats),     false},
    };
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_grid_set_row_spacing(GTK_GRID(grid),10);
    int n=sizeof(btns)/sizeof(btns[0]);
    for(int i=0;i<n;i++){
        GtkWidget *b=make_btn(btns[i].label,btns[i].cb,NULL,btns[i].danger);
        gtk_widget_set_hexpand(b,TRUE);
        gtk_grid_attach(GTK_GRID(grid),b,i%3,i/3,1,1);
    }
    gtk_box_pack_start(GTK_BOX(box),grid,FALSE,FALSE,0);
    return box;
}

static GtkWidget* build_rec_page(void){
    GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    GtkWidget *t=gtk_label_new("Recursion Module");
    gtk_style_context_add_class(gtk_widget_get_style_context(t),"module-title");
    gtk_widget_set_halign(t,GTK_ALIGN_START);
    GtkWidget *s=gtk_label_new("MODULE IV  ·  RECURSIVE ALGORITHMS");
    gtk_style_context_add_class(gtk_widget_get_style_context(s),"module-subtitle");
    gtk_widget_set_halign(s,GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box),t,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),s,FALSE,FALSE,0);
    GtkWidget *sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(sep),"gold-sep");
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,4);

    struct { const char *label; GCallback cb; bool danger; } btns[] = {
        {"Count Occurrences",       G_CALLBACK(on_rec_count),      false},
        {"Palindrome Check",        G_CALLBACK(on_rec_palindrome),  false},
        {"Name Permutations",       G_CALLBACK(on_rec_permutation), false},
        {"Distinct Subsequences",   G_CALLBACK(on_rec_distinct),   false},
        {"Remove Occurrences",      G_CALLBACK(on_rec_remove),      true},
    };
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_grid_set_row_spacing(GTK_GRID(grid),10);
    int n=sizeof(btns)/sizeof(btns[0]);
    for(int i=0;i<n;i++){
        GtkWidget *b=make_btn(btns[i].label,btns[i].cb,NULL,btns[i].danger);
        gtk_widget_set_hexpand(b,TRUE);
        gtk_grid_attach(GTK_GRID(grid),b,i%2,i/2,1,1);
    }
    gtk_box_pack_start(GTK_BOX(box),grid,FALSE,FALSE,0);
    return box;
}

// ═══════════════════════════════════════════════════════════════
//  MAIN WINDOW BUILDER
// ═══════════════════════════════════════════════════════════════

static void on_search_activate(GtkEntry *e, gpointer u){
    ll_search_by_name(gtk_entry_get_text(e));
}

static void build_ui(GtkApplication *app){
    // Apply CSS
    GtkCssProvider *css=gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,ALGERIA_CSS,-1,NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Main window
    g_main_window=gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(g_main_window),"Algeria History Database");
    gtk_window_set_default_size(GTK_WINDOW(g_main_window),1100,700);
    gtk_window_set_position(GTK_WINDOW(g_main_window),GTK_WIN_POS_CENTER);

    // Root vbox
    GtkWidget *root=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_container_add(GTK_CONTAINER(g_main_window),root);

    // ── Header ──────────────────────────────────────────────────
    GtkWidget *header=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_style_context_add_class(gtk_widget_get_style_context(header),"app-header");
    gtk_widget_set_size_request(header,-1,56);

    GtkWidget *hbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
    gtk_widget_set_valign(hbox,GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(hbox,8);
    GtkWidget *htitle=gtk_label_new("ALGERIA HISTORY DATABASE");
    gtk_style_context_add_class(gtk_widget_get_style_context(htitle),"app-title");
    gtk_widget_set_halign(htitle,GTK_ALIGN_START);
    GtkWidget *hsub=gtk_label_new("DYNAMIC DATA STRUCTURES  ·  NSCS 2025–2026");
    gtk_style_context_add_class(gtk_widget_get_style_context(hsub),"app-subtitle");
    gtk_widget_set_halign(hsub,GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(hbox),htitle,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbox),hsub,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(header),hbox,TRUE,TRUE,0);

    // Search bar in header
    g_search_entry=gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(g_search_entry),"Quick search...");
    gtk_widget_set_size_request(g_search_entry,220,-1);
    gtk_widget_set_valign(g_search_entry,GTK_ALIGN_CENTER);
    gtk_widget_set_margin_end(g_search_entry,16);
    g_signal_connect(g_search_entry,"activate",G_CALLBACK(on_search_activate),NULL);
    gtk_box_pack_end(GTK_BOX(header),g_search_entry,FALSE,FALSE,0);

    gtk_box_pack_start(GTK_BOX(root),header,FALSE,FALSE,0);

    // ── Body (sidebar + content) ─────────────────────────────────
    GtkWidget *body=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_box_pack_start(GTK_BOX(root),body,TRUE,TRUE,0);

    // Sidebar
    GtkWidget *sidebar=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar),"sidebar");
    gtk_widget_set_size_request(sidebar,220,-1);

    // Sidebar ornament top
    GtkWidget *logo_lbl=gtk_label_new("☽ ✦ ☽");
    gtk_style_context_add_class(gtk_widget_get_style_context(logo_lbl),"app-title");
    gtk_widget_set_margin_top(logo_lbl,20);
    gtk_widget_set_margin_bottom(logo_lbl,4);
    gtk_box_pack_start(GTK_BOX(sidebar),logo_lbl,FALSE,FALSE,0);

    GtkWidget *sec_lbl=gtk_label_new("NAVIGATION");
    gtk_style_context_add_class(gtk_widget_get_style_context(sec_lbl),"sidebar-title");
    gtk_widget_set_margin_top(sec_lbl,16);
    gtk_box_pack_start(GTK_BOX(sidebar),sec_lbl,FALSE,FALSE,0);

    GtkWidget *ssep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(ssep),"gold-sep");
    gtk_widget_set_margin_start(ssep,16); gtk_widget_set_margin_end(ssep,16);
    gtk_box_pack_start(GTK_BOX(sidebar),ssep,FALSE,FALSE,8);

    // Nav buttons
    struct { const char *label; const char *page; } nav_items[] = {
        {"⌂  Welcome",              "welcome"},
        {"⊞  Linked Lists",         "linkedlists"},
        {"▦  Stack Module",         "stack"},
        {"⌥  Binary Search Tree",   "bst"},
        {"↺  Recursion",            "recursion"},
    };
    int nav_n = sizeof(nav_items)/sizeof(nav_items[0]);
    GtkWidget **nav_btns = g_new0(GtkWidget*, nav_n);
    GList *btn_list = NULL;

    for(int i=0;i<nav_n;i++){
        nav_btns[i]=gtk_button_new_with_label(nav_items[i].label);
        gtk_style_context_add_class(gtk_widget_get_style_context(nav_btns[i]),"nav-btn");
        gtk_button_set_relief(GTK_BUTTON(nav_btns[i]),GTK_RELIEF_NONE);
        btn_list=g_list_append(btn_list,nav_btns[i]);
        gtk_box_pack_start(GTK_BOX(sidebar),nav_btns[i],FALSE,FALSE,0);
    }
    // set active first
    gtk_style_context_add_class(gtk_widget_get_style_context(nav_btns[0]),"active");

    // Sidebar bottom info
    GtkWidget *spacer=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_vexpand(spacer,TRUE);
    gtk_box_pack_start(GTK_BOX(sidebar),spacer,TRUE,TRUE,0);

    GtkWidget *ssep2=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(ssep2),"gold-sep");
    gtk_widget_set_margin_start(ssep2,16); gtk_widget_set_margin_end(ssep2,16);
    gtk_box_pack_start(GTK_BOX(sidebar),ssep2,FALSE,FALSE,0);
    GtkWidget *ver=gtk_label_new("v1.0  ·  Algeria NSCS");
    gtk_style_context_add_class(gtk_widget_get_style_context(ver),"func-desc");
    gtk_widget_set_margin_bottom(ver,12);
    gtk_box_pack_start(GTK_BOX(sidebar),ver,FALSE,FALSE,4);

    gtk_box_pack_start(GTK_BOX(body),sidebar,FALSE,FALSE,0);

    // ── Content pane (stack + output) ────────────────────────────
    GtkWidget *content_vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_hexpand(content_vbox,TRUE);
    gtk_box_pack_start(GTK_BOX(body),content_vbox,TRUE,TRUE,0);

    // Module pages
    g_stack_pages=gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(g_stack_pages),
        GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(g_stack_pages),250);
    gtk_widget_set_vexpand(g_stack_pages,FALSE);

    GtkWidget *page_wrap[5];
    const char *page_names[]={"welcome","linkedlists","stack","bst","recursion"};
    GtkWidget *pages[5];
    pages[0]=build_welcome_page();
    pages[1]=build_ll_page();
    pages[2]=build_stack_page();
    pages[3]=build_bst_page();
    pages[4]=build_rec_page();

    for(int i=0;i<5;i++){
        page_wrap[i]=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
        gtk_style_context_add_class(gtk_widget_get_style_context(page_wrap[i]),"content-area");
        gtk_widget_set_margin_start(pages[i],4);
        gtk_box_pack_start(GTK_BOX(page_wrap[i]),pages[i],TRUE,TRUE,0);
        gtk_stack_add_named(GTK_STACK(g_stack_pages),page_wrap[i],page_names[i]);
    }
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack_pages),"welcome");

    // Wire nav buttons now that g_stack_pages exists
    for(int i=0;i<nav_n;i++){
        NavData *nd=g_new0(NavData,1);
        nd->btn=nav_btns[i];
        nd->all_btns=btn_list;
        nd->page_name=nav_items[i].page;
        g_signal_connect(nav_btns[i],"clicked",G_CALLBACK(on_nav_click),nd);
    }

    gtk_box_pack_start(GTK_BOX(content_vbox),g_stack_pages,FALSE,FALSE,0);

    // Separator
    GtkWidget *mid_sep=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_style_context_add_class(gtk_widget_get_style_context(mid_sep),"gold-sep");
    gtk_box_pack_start(GTK_BOX(content_vbox),mid_sep,FALSE,FALSE,0);

    // Output label
    GtkWidget *out_lbl_row=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,8);
    gtk_widget_set_margin_start(out_lbl_row,16);
    gtk_widget_set_margin_top(out_lbl_row,8);
    GtkWidget *out_lbl=gtk_label_new("OUTPUT TERMINAL");
    gtk_style_context_add_class(gtk_widget_get_style_context(out_lbl),"sidebar-title");
    gtk_box_pack_start(GTK_BOX(out_lbl_row),out_lbl,FALSE,FALSE,0);
    GtkWidget *clr_btn=gtk_button_new_with_label("✕ Clear");
    gtk_style_context_add_class(gtk_widget_get_style_context(clr_btn),"action-btn");
    gtk_widget_set_margin_end(clr_btn,16);
    g_signal_connect_swapped(clr_btn,"clicked",G_CALLBACK(output_clear),NULL);
    gtk_box_pack_end(GTK_BOX(out_lbl_row),clr_btn,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(content_vbox),out_lbl_row,FALSE,FALSE,0);

    // Output textview
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_widget_set_margin_start(scroll,12);
    gtk_widget_set_margin_end(scroll,12);
    gtk_widget_set_margin_bottom(scroll,4);

    g_output_view=gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_output_view),FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(g_output_view),FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(g_output_view),GTK_WRAP_WORD_CHAR);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_output_view),"output-text");
    gtk_widget_set_size_request(g_output_view,-1,220);

    gtk_container_add(GTK_CONTAINER(scroll),g_output_view);
    gtk_style_context_add_class(gtk_widget_get_style_context(scroll),"output-frame");
    gtk_box_pack_start(GTK_BOX(content_vbox),scroll,TRUE,TRUE,0);

    // Status bar
    g_status_bar=gtk_label_new("  Ready");
    gtk_style_context_add_class(gtk_widget_get_style_context(g_status_bar),"status-bar");
    gtk_widget_set_halign(g_status_bar,GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root),g_status_bar,FALSE,FALSE,0);

    gtk_widget_show_all(g_main_window);

    // Load database on startup
    load_database();
    // Welcome message in output
    output_append("╔══════════════════════════════════════════════════════════╗\n");
    output_append("║       ALGERIA HISTORY DATABASE — INITIALIZED            ║\n");
    output_append("║       Algorithms & Dynamic Data Structures               ║\n");
    output_append("║       NSCS  ·  2025–2026                                 ║\n");
    output_append("╚══════════════════════════════════════════════════════════╝\n\n");
    output_append("  Navigate using the sidebar to manage personalities,\n");
    output_append("  events, stacks, binary search trees and recursion.\n\n");
    output_append("  ✦ Database file: database.txt\n");
}

// ═══════════════════════════════════════════════════════════════
//  ENTRY POINT
// ═══════════════════════════════════════════════════════════════

static void on_activate(GtkApplication *app, gpointer user_data){
    build_ui(app);
}

int main(int argc, char **argv){
    if(argc>1) strncpy(g_db_path,argv[1],511);
    GtkApplication *app=gtk_application_new("dz.nscs.algeria_history",G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app,"activate",G_CALLBACK(on_activate),NULL);
    int status=g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return status;
}
