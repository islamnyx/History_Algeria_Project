#ifndef STACKS_H
#define STACKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lists_Queues.h"
#include "structs.h"


//!________ Helper Functions ___________

date stringToDate(char *str);
int countWords(char *definition);
bool isOverlapping(date dob1, date dod1, date dob2, date dod2);
void insertAtBottom(TStack *stk, Tlist *node);

//!________ Stacks Basic Operations ___________

void push(TStack *stk, char *name, char *definition, date dob, date dod);
Tlist* pop(TStack *stk);
bool isEmpty(TStack *stk);


//!________ Stacks Basic Operations ___________

TStack* toStack(Tlist *merged);
TStack* getInfoPersonality(TStack *stk, char *name);
TStack* sortNameStack(TStack *s);
TStack* deleteName(TStack *stk, char *name);
TStack* updateStack(TStack *stk, char *name, char *def, char *DoB, char *DoD);
TQueue* stackToQueue(TStack *stk);
TBilist* stackToList(TStack *stk);
TStack* addNameStack(TStack *stk, char *name, char *definition, char *DoB, char *DoD);
TStack* definitionStack(TStack *stk);
void pronunciationStack(TStack *stk, TStack **shortStack, TStack **longStack);
char* getSmallest(TStack *stk);
void continuousSearch(TStack *stk);
bool isPersonalityKilled(char *word);
TStack* recRevStack(TStack *stk);

#endif



