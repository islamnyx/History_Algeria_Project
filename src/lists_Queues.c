#include <stdio.h>
#include "structs.h"
#include <string.h>
#include <stdlib.h>


void printList(Tlist *head){
    if(head == NULL){
        printf("No Personalities found!\n");
        return;  
    }
    
   Tlist *current = head;
   int count = 1;
   printf("======History of Algeria Database=====\n");
   while(current != NULL){
     printf("%d. Name:  %s\n", count++ , current->name);
     printf("    Bio: %s\n", current->definition);
     printf("==================================\n");
     current = current->next;
   }


}

Tlist* getPersonality(FILE *f){
 char buffer[500];
 Tlist *head = NULL;
 Tlist *tail = NULL;

if (f == NULL)  return NULL;

  while (fgets(buffer, sizeof(buffer), f) != NULL){
    char  tempName[100];
    char  tempDef[250];

     if(sscanf(buffer , "%[^=]=%[^=]", tempName, tempDef) == 2){
        Tlist *newnode = (Tlist*)malloc(sizeof(Tlist));
        if(newnode == NULL){
            printf("Memory allocation failed!");
            break;
        }

        strcpy(newnode->name, tempName);
        strcpy(newnode->definition, tempDef);

        newnode->dod.day = 0;
        newnode->dod.month = 0;
        newnode->dod.year = 0;

        newnode->dob.day = 0;
        newnode->dob.month = 0;
        newnode->dob.year = 0;

        newnode->next = NULL;

        if(head == NULL){
          head = newnode;
          tail = newnode;  

        }else{
            tail->next = newnode;
            tail = newnode;
        }
     }
  }
    return head ;

}

Tlist* getDatePersonality(FILE *f){
    
     char buffer[500];
     Tlist *head = NULL;
     Tlist *tail = NULL;

     if (f == NULL)  return NULL;

     rewind(f);
      while (fgets(buffer, sizeof(buffer), f) != NULL){
    char  tempName[100];
    int dob, dod;

     if(sscanf(buffer , "%[^=]=%[^=]=%d=%d", tempName, &dob ,&dod) == 3){
        Tlist *newnode = (Tlist*)malloc(sizeof(Tlist));
        if(newnode == NULL){
            printf("Memory allocation failed!");
            break;
        }
        strcpy(newnode->name, tempName);
        newnode->dod.day = 0;
        newnode->dod.month = 0;
        newnode->dod.year = 0;

        newnode->dob.day = 0;
        newnode->dob.month = 0;
        newnode->dob.year = 0;
        newnode->next = NULL;

        if(head == NULL){
          head = newnode;
          tail = newnode;  

        }else{
            tail->next = newnode;
            tail = newnode;
        }
     }
  }
    return head ;
}

void getinfoByDates(Tlist *s , Tlist *dob){
    int found = 0;
  int  d , m ,y;
   printf("Enter Date of birth (DD MM YYYY): ");
   scanf("%d %d %d", &d ,&m , &y);

   Tlist *currDate = dob;

      while(currDate != NULL){
            if(currDate->dob.day == d &&
               currDate->dob.month == m &&
               currDate->dob.year == y ){
                 Tlist *currBio = s;

                 while(currBio != NULL){
                    if(strcmp(currBio->name , currDate->name) == 0){
                          printf("\n==============================");
                          printf("\nName     : %s" , currBio->name);
                          printf("\nDate of birth   : %d/%d/%d", currBio->dob.day , currBio->dob.month , currBio->dob.year);
                          printf("\nDefinition  : %s", currBio->definition);
                          printf("\n=============================\n");
                          found = 1;
                          break;
                    }
                  currBio = currBio->next;
                 }
            }
             currDate = currDate->next;
      }

  if(!found){
   printf("\n no historical figure in our records was born in %d/%d/%d.\n",d , m , y );
  }

}

void getinfoByDates2(Tlist *s , Tlist *dod){
    int found = 0;
  int  d , m ,y;
   printf("Enter Date of death (DD MM YYYY): ");
   scanf("%d %d %d", &d ,&m , &y);

   Tlist *currDeath = dod;

      while(currDeath != NULL){
            if(currDeath->dod.day == d &&
               currDeath->dod.month == m &&
               currDeath->dod.year == y ){
                 Tlist *currBio = s;

                 while(currBio != NULL){
                    if(strcmp(currBio->name , currDeath->name) == 0){
                          printf("\n==============================");
                          printf("\nName     : %s" , currBio->name);
                          printf("\nDate of birth   : %d/%d/%d", currBio->dob.day , currBio->dob.month , currBio->dob.year);
                          printf("\nDefinition  : %s", currBio->definition);
                          printf("\n=============================\n");
                          found = 1;
                          break;
                    }
                  currBio = currBio->next;
                 }
            }
             currDeath = currDeath->next;
      }

  if(!found){
   printf("\n no historical figure in our records die on %d/%d/%d.\n",d , m , y );
  }

}

Tlist* sortWord(Tlist *syn){
   if(syn == NULL) return NULL;


   int swapped ;
   Tlist *ptr1;
   Tlist *lptr = NULL;

 do {
   swapped = 0;
   ptr1 = syn;

   while(ptr1->next != lptr){

    if(strcmp(ptr1->name,ptr1->next->name)> 0){

         //1. Swap Name 
         char temp[100];
         strcpy(temp , ptr1->name);
         strcpy(ptr1->name, ptr1->next->name);
         strcpy(ptr1->next->name,temp);

        //2. Swap Dates of birth
         date tempDob = ptr1->dob;
                ptr1->dob = ptr1->next->dob;
                ptr1->next->dob = tempDob;

        //3. Swap Dates of Dead
         date tempDod = ptr1->dod;
                ptr1->dod = ptr1->next->dod;
                ptr1->next->dod = tempDod;

        //4. Swap Definition 
        char tempDef[250];
        strcpy(tempDef, ptr1->definition);
        strcpy(ptr1->definition, ptr1->next->definition);
        strcpy(ptr1->next->definition, tempDef);

        swapped = 1;
    }

    ptr1 = ptr1->next;
   }
   lptr = ptr1;
 } while (swapped);

 return syn;

}

//At the bottom, the while(swapped) checks: "Did the sensor go off?"
//If Yes (1): We must go back to the top and check again, because that swap might have created a new alphabetical conflict somewhere else.
//If No (0): We finished a whole trip without touching anything. The list is sorted!











