#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structs.h"
#include "lists_Queues.h"


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


int main(){
  
   FILE *f = fopen("database.txt", "r"); 
   if (f == NULL) { printf("Failed to open database.txt\n"); 
    return 1; } 
    Tlist *head = getPersonality(f); 
   fclose(f); 
   printList(head);





    return 0;

}


