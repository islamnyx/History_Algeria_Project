#include <stdio.h>

typedef struct {
    int day;
    int month;
    int year;
} date;

typedef struct node {
   char name[100];
   char definition[250];
   date dob; //date of birth
   date dod; //date of death
   struct node* next;
}Tlist;

typedef struct binode {
   char name[100];
   char definition[250];
   date dob; //date of birth
   date dod; //date of death      
   struct binode* next;
   struct binode* prev;
} TBilist;

typedef struct QNode {
    char name[100];
    char definition[250];
    date dob;
    date dod;
    struct QNode* next;
}QNode;

typedef struct {
   QNode *head;
   QNode *tail; 
} TQueue;






