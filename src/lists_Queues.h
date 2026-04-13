#ifdef LISTS_QUEUES_H
#define LISTS_QUEUES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"


//Function Prototypes 
void printList(Tlist *head);
Tlist* getPersonality(FILE *f);
Tlist* getDatePersonality(FILE *f);
void getinfoByDates(Tlist *s , Tlist *dob);
void getinfoByDates2(Tlist *s , Tlist *dob);
Tlist* sortWord(Tlist *syn);
Tlist* sortWord2(Tlist *syn);
Tlist* sortPersonality(Tlist* syn);
Tlist* deletePersonality(FILE *f , Tlist *s , Tlist *a , char *name );
Tlist* UpdatePersonality(FILE *f , Tlist *s , Tlist *a , char *name , char *definition , 
                                 char *dob , char *dod )
Tlist* similarPersonality(Tlist *s , char *word);
Tlist* countPersonality(Tlist *s , date *prt);
Tlist* addPersonality(Tlist *s , Tlist *a , char *name ,char *definition,  char *dob , char *dod);
Tlist* addEvents(Tlist *b , char *nameEvente , char *date);

TQueue* toQueue(Tlist *merged);
TQueue* sName(Tlist *s);



void enqueue(TQueue *q , char *name , char *definition , date dob , date dod);
TBilist* insertBilist(TBilist *head, char *name, char *definition, date dob, date dod);
#endif

