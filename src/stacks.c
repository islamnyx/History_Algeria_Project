#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lists_Queues.h"
#include "structs.h"




void push(TStack *stk , char *name , char *definition , date dob , date dod ){
    Tlist *newNode = (Tlist*)malloc(sizeof(Tlist));
    strcpy(newNode->name , name);
    strcpy(newNode->definition,definition );
    newNode->dob = dob;
    newNode->dod = dod;
    newNode->next = stk->top;
    stk->top = newNode ;

}
bool isEmpty(TStack *stk){
    return stk->top == NULL;
}

Tlist* pop(TStack *stk){
    if(stk->top == NULL) return NULL;
    Tlist *temp = stk->top;
    stk->top = stk->top->next;
    temp->next = NULL;
    return temp;

}

TStack* toStack(Tlist *merged ){
    TStack *stk = (TStack*)malloc(sizeof(TStack));
    stk->top = NULL;

    Tlist *current = merged;
    while(current != NULL){
        push(stk, current->name , current->definition , current->dob , current->dod);
        current = current->next;
    }

    return stk;

}

TStack* getInfoPersonality(TStack *stk , char *name ){
    TStack *result = (TStack*)malloc(sizeof(TStack));
    result->top = NULL;

    Tlist *current  = stk->top;
    while(current != NULL){
        if(strcmp(current->name , name)== 0){
            push(result , current->name , current->definition , current->dob , current->dod);
            return result;
        }
        current = current->next;
    }

   return NULL;
}

TStack* sortNameStack(TStack *s){
   TStack *sorted = (TStack*)malloc(sizeof(TStack));
   sorted->top = NULL;

   TStack *temp = (TStack*)malloc(sizeof(TStack));

    while(!isEmpty(temp)){

       Tlist *current = temp->top;
       Tlist *smallest = temp->top;

       while(current != NULL){
        if(strcmp(current->name , smallest->name ) < 0){
            smallest = current;
        }
        current = current ->next;
       }

       push(sorted , smallest->name , smallest->definition , smallest->dob , smallest->dod);
          

       Tlist *prev = NULL;
       Tlist *cur = temp->top;
       while(cur != smallest){
        prev = cur;
        cur = cur->next;

       }
       if(prev == NULL){
        temp->top = cur->next;
       } else{
        prev->next = cur->next;
       }
       free(cur);
    }
    return sorted;
}

//! This function takes a stack and a name, removes the node with that name from the stack, 
//! and returns the updated stack.

TStack* deleteName(TStack *stk , char *name){
     Tlist *prev = NULL;
     Tlist *current = stk->top;

     while(current != NULL){
         if(strcmp(current->name, name)== 0){
            if(prev == NULL){
                stk->top = current->next;

            }else{
                prev->next = current->next;

            }

            free(current);
            return stk;

         }
         prev = current;
         current = current->next;
     }

     return stk;

}

//! helper function to dod and dob 

date stringToDate(char *str){
    date d;
    sscanf(str, "%d/%d/%d", &d.day , &d.month , &d.year);
    return d;
}

TStack* updateStack(TStack *stk, char *name , char *def , char *DoB , char *DoD){
      Tlist *current = stk->top;



      while(current != NULL){
        if(strcmp(current->name , name) == 0){
             strcpy(current->definition , def);
             current->dob = stringToDate(DoB);
             current->dod = stringToDate(DoD);
             return stk;
        }
        current = current->next;

      }
      return stk;
}


TQueue* stackToQueue(TStack *stk){
    TQueue *q = (TQueue*)malloc(sizeof(TQueue));
    q->head = NULL;
    q->tail = NULL;

    Tlist *current = stk->top;

    while(current != NULL){
        enqueue(q, current->name , current->definition , current->dob , current->dod);
        current = current->next;
    }

    return q;
}

TBilist* stackToList(TStack *stk){
    TBilist *head = NULL;

    Tlist *current = stk->top;

    while(current != NULL){
        head = insertBilist(head, current->name , current->definition , current->dob , current->dod);
    }
    return head ;
}


TStack* addNameStack(TStack *stk , char *name, char *definition , char *DoB, char *DoD){
    Tlist *newNode = (Tlist*)malloc(sizeof(Tlist));
    strcpy(newNode->name, name);
    strcpy(newNode->definition, definition);
    newNode->dob = stringToDate(DoB);
    newNode->dod = stringToDate(DoD);
    newNode->next = NULL;

    if(stk->top == NULL || strcmp(name , stk->top->name) < 0){
        newNode->next = stk->top;
        stk->top = newNode;
        return stk;
    }

    Tlist *current = stk->top;
    while(current->next != NULL && strcmp(name, current->next->name) > 0){
       current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;

   return stk;
}

//! helper Function 

int countWords(char *definition){
    int count = 0;
    char temp[250];

    strcpy(temp , definition);
    char *token = strtok(temp, " ");
    while(token != NULL){
        count++;
        token = strtok(NULL, " ");
 
    }
    return count ;
}

TStack* definitionStack(TStack *stk){
   TStack *sorted = (TStack*)malloc(sizeof(TStack));
   sorted->top = NULL;

   TStack *temp = (TStack*)malloc(sizeof(TStack));
   temp->top = stk->top;

   while(!isEmpty(temp)){
      Tlist *current = temp->top;
      Tlist *smallest = temp->top;

      while(current != NULL){
        if(countWords(current->definition) < countWords(smallest->definition)){
            smallest = current;
        }
        current = current->next;
      }
      push(sorted , smallest->name , smallest->definition , smallest->dob ,smallest->dod);
      Tlist *prev =NULL;
      Tlist *cur = temp->top;
      while(cur != smallest){
        prev = cur;
        cur = cur->next;
      }
      if(prev == NULL){
        temp->top = cur->next;
      } else {
        prev->next = cur->next;
      }
      free(cur);
   }

   return sorted;

}

void pronunciationStack(TStack *stk , TStack **shortStack , TStack **longStack){
    *shortStack = (TStack*)malloc(sizeof(TStack));
    (*shortStack)->top = NULL;

    *longStack = (TStack*)malloc(sizeof(TStack));
    (*longStack)->top = NULL;

    Tlist *current = stk->top;
    while(current != NULL){
        if(countWords(current->definition) <= 5){
            push(*shortStack, current->name , current->definition , current->dob , current->dod);
        } else {
            push(*longStack , current->name , current->definition , current->dob , current->dod);
        }
        current = current->next;
    }
}

char* getSmallest(TStack *stk){
    if(stk->top == NULL) return NULL;

    Tlist *current = stk->top;
    char *smallest = stk->top->definition;

    while(current != NULL){
        if(countWords(current->definition) < countWords(smallest)){
            smallest = current->definition;
        }
        current = current->next;
    }
    return smallest;
}


bool isOverlapping(date dob1 ,date dod1 ,date dob2 , date dod2){

    int start1 = dob1.year * 10000 + dob1.month * 100 + dob1.day;
    int end1   = dod1.year * 10000 + dod1.month * 100 + dod1.day;
    int start2 = dob2.year * 10000 + dob2.month * 100 + dob2.day;
    int end2   = dod2.year * 10000 + dod2.month * 100 + dod2.day;

    return (start1 <= end2 && start2 <= end1);
}

void continuousSearch(TStack *stk){
     Tlist *current = stk->top;

   while(current != NULL){
    Tlist *next = current->next;
       while(next != NULL){
        if(isOverlapping(current->dob, current->dod, next->dob, next->dod)){
                printf("Overlapping personalities:\n");
                printf("Name: %s | DoB: %02d/%02d/%04d | DoD: %02d/%02d/%04d\n",
                    current->name,
                    current->dob.day, current->dob.month, current->dob.year,
                    current->dod.day, current->dod.month, current->dod.year);
                printf("Name: %s | DoB: %02d/%02d/%04d | DoD: %02d/%02d/%04d\n",
                    next->name,
                    next->dob.day, next->dob.month, next->dob.year,
                    next->dod.day, next->dod.month, next->dod.year);
                printf("-----------------------------------\n");
        }
        next = next->next;

       }
      current = current->next;
   }

}


bool isPersonalityKilled(char *word){
    if(strstr(word , "killed") != NULL){
        return true;
    }
    return false ;
}