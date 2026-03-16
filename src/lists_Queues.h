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


#endif

