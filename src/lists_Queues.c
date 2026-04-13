#include <stdio.h>
#include "structs.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



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


Tlist* sortWord2(Tlist* syn){
      if(syn == NULL) return NULL;


   int swapped ;
   Tlist *ptr1;
   Tlist *lptr = NULL;

 do {
   swapped = 0;
   ptr1 = syn;

   while(ptr1->next != lptr){

    if(strlen(ptr1->name) > strlen(ptr1->next->name)){

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

//same logique with sortword2 + change the condition inside if
//sortword  : strcmp(ptr1->name,ptr1->next->name)> 0
//sortword2 : strlen(ptr1->name) > strlen(ptr1->next->name)

Tlist* sortPersonality(Tlist* syn){
  
      if(syn == NULL) return NULL;


   int swapped ;
   Tlist *ptr1;
   Tlist *lptr = NULL;

 do {
   swapped = 0;
   ptr1 = syn;

   while(ptr1->next != lptr){
      int age1 = ptr1->dod.year - ptr1->dob.year;
      int age2 = ptr1->next->dod.year - ptr1->next->dob.year;

    if(age1 > age2){

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


Tlist* deletePersonality(FILE *f , Tlist *s , Tlist *a , char *name ){

    // for linked list s
  Tlist *curr = s , *prev = NULL;
   while(curr!=NULL){
    if(strcmp(curr->name , name ) == 0){
      if(prev == NULL){
        s = curr->next;
      }else{
         prev->next = curr->next;
         free(curr);
         break;
      } 
      prev = curr;
      curr = curr->next;
    }
   }

    // for linked list a
  Tlist *currA = a , *prevA = NULL;
   while(currA!=NULL){
    if(strcmp(currA->name , name ) == 0){
      if(prevA == NULL){
        s = currA->next;
      }else{
         prevA->next = currA->next;
         free(currA);
         break;
      } 
      prevA = currA;
      currA = currA->next;
    }
   }

   // Updating the file 

  fclose(f);
  f = fopen("database.txt" , "w");
  

  Tlist *temp = s;
    while (temp != NULL) {
        // Writing it  in the format: Name=Definition=DoB=DoD
        fprintf(f, "%s=%s=%d/%d/%d=%d/%d/%d\n", 
                temp->name, temp->definition, 
                temp->dob.day, temp->dob.month, temp->dob.year,
                temp->dod.day, temp->dod.month, temp->dod.year);
        temp = temp->next;
    }
  
  return s;
}

Tlist* UpdatePersonality(FILE *f , Tlist *s , Tlist *a , char *name , char *definition , 
                                 char *dob , char *dod ){
  int d1 , m1 , y1 , d2 ,m2 ,y2;
   Tlist *currS = s;
   Tlist *currA = a;

  sscanf(dob , "%d/%d/%d/" , &d1 , &m1 , &y1);
  sscanf(dod , "%d/%d/%d/" , &d2 , &m2 , &y2);

   //Updating the main list (s)
    while(currS != NULL){
      if(strcmp(currS->name , name) == 0){
         strcpy(currS->definition, definition);
            currS->dob = (date){d1, m1, y1};
            currS->dod = (date){d2, m2, y2};
            break;
    }
    currS = currS->next;
  }
  
  //Updating the Date list (a)  
    while(currA != NULL){
      if(strcmp(currA->name , name) == 0){
         strcpy(currA->definition, definition);
            currA->dob = (date){d1, m1, y1};
            currA->dod = (date){d2, m2, y2};
            break;
    }
    currA = currA->next;
  }

  fclose(f);
  f = fopen("database.txt", "w");
  if(f == NULL) return s;

 Tlist *temp = s;
 while(temp != NULL){
  fprintf(f, "%s=%s=%02d/%02d/%d=%02d/%02d/%d\n", 
                temp->name, temp->definition, 
                temp->dob.day, temp->dob.month, temp->dob.year,
                temp->dod.day, temp->dod.month, temp->dod.year);
        temp = temp->next;
    }
  return s;

}


//! This function is essentially a Filter. It scans your main list and 
//! picks out anyone who "matches" a specific year provided by the user.

Tlist* similarPersonality(Tlist *s , char *word){
   Tlist *resultList = NULL;
   int targetYear = atoi(word);
   //! atoi()  converts string into integer 

   Tlist *curr = s;
   while(curr != NULL){
     if(curr->dob.year == targetYear || curr->dod.year == targetYear ){
         Tlist *newnode = (Tlist*)malloc(sizeof(Tlist));
               strcpy(newnode->name, curr->name);
               strcpy(newnode->definition, curr->definition);
               newnode->dob = curr->dob;
               newnode->dod = curr->dod;
                
               // here we are inserting at the beginning 
               newnode->next = resultList;
               resultList = newnode;
     }
     curr = curr->next;
   } 
   return resultList;
}



Tlist* countPersonality(Tlist *s , date *prt){
   Tlist *results = NULL;
   Tlist *curr = s;
   int matchBirth;
   int matchDeath;

   while (curr != NULL){

       matchBirth = (curr->dob.day == prt->day && 
                          curr->dob.month == prt->month && 
                          curr->dob.year == prt->year);
                        
       matchDeath = (curr->dod.day == prt->day && 
                          curr->dod.month == prt->month && 
                          curr->dod.year == prt->year);
    
       if(matchBirth || matchDeath){

   
            Tlist *newNode = (Tlist*)malloc(sizeof(Tlist));
            
           
            strcpy(newNode->name, curr->name);
            strcpy(newNode->definition, curr->definition);
            newNode->dob = curr->dob;
            newNode->dod = curr->dod;

           
            newNode->next = results;
            results = newNode;
       }
       curr = curr->next;
   }
   return results;
}

int isPalindrome(char *str){

   int left = 0;
   int right = strlen(str) - 1;

  while(right > left){
    if(tolower(str[left] != tolower(str[right]))){
      return 0;
    }
    left++;
    right--;
  }
return 1;
}

//! TList* palindromeName(TList *s)
Tlist* palindromeName(Tlist *s){

  Tlist *newlist = NULL;
  Tlist *current = s;

  while (current != NULL){

    if(isPalindrome(current->name)){

      //create new node 
      Tlist *newnode = (Tlist*)malloc(sizeof(Tlist));
      strcpy(newnode->name, current->name);
      newnode->next = NULL;

      //Sorting 
      if(newlist == NULL || strcmp(newnode->name , newlist->name)< 0){
        newnode->next = newlist;
        newlist = newnode;
        //strcmp(newnode->name , newlist->name)< 0
        /*
        If the first string is smaller (comes first in the alphabet)
                   , it returns a negative number (<0).
        */

      }else{
        Tlist *temp = newlist;
        while(temp->next != NULL || strcmp(temp->next->name , newnode->name) < 0 ){
            temp = temp->next;
         }

        newnode->next = temp->next;
        temp->next = newnode;

    }
  }
  current = current->next;   
 }
return newlist;
}

//! TList* mergeNodes(TList *s, TList *a)

TBilist* mergeNodes(Tlist *s , Tlist *a){
  if (s == NULL || a == NULL) return NULL;
  TBilist *head = NULL;
  TBilist *tail = NULL;
  Tlist *currS = s;
  Tlist *currA = a;

  while(currS != NULL && currA != NULL){

    TBilist *newnode = (TBilist*)malloc(sizeof(TBilist));
      //copy data from the Personality list
    strcpy(newnode->name , currS->name);
    strcpy(newnode->definition , currS->definition);
      //copy data from the Dates list 
    newnode->dob = currA->dob;
    newnode->dod = currA->dod;

    newnode->next = NULL;

    if(head == NULL){
      newnode->prev = NULL;
      head = newnode;
      tail = newnode;
    }else{
      tail->next = newnode;
      newnode->prev = tail;
      tail = newnode;
    }

    currS = currS->next;
    currA = currA->next;
  }
  

return head;

}

//! TList* merge2Nodes(TList *s, TList *a)
Tlist* merge2Nodes(Tlist *s , Tlist *a){
  if (s == NULL || a == NULL) return NULL;
  Tlist *head = NULL;
  Tlist *tail = NULL;
  Tlist *currS = s;
  Tlist *currA = a;

  while(currS != NULL && currA != NULL){

    Tlist *newnode = (Tlist*)malloc(sizeof(Tlist));
      //copy data from the Personality list
    strcpy(newnode->name , currS->name);
    strcpy(newnode->definition , currS->definition);
      //copy data from the Dates list 
    newnode->dob = currA->dob;
    newnode->dod = currA->dod;

    if(head == NULL){
      head = newnode;
      tail = newnode;
      newnode->next = head;
    }else{
      tail->next = newnode;
      tail = newnode;
      tail->next = head;
    }

    currS = currS->next;
    currA = currA->next;
  }

return head;
}


Tlist* addPersonality(Tlist *s , Tlist *a , char *name ,char *definition,  char *dob , char *dod){
  
  // create new node
  Tlist *newnode = (Tlist*)malloc(sizeof(Tlist));
  strcpy(newnode->name , name );
  strcpy(newnode->definition , definition);
  sscanf(dob , "%d/%d/%d" , &newnode->dob.day , &newnode->dob.month , &newnode->dob.year);
  sscanf(dod , "%d/%d/%d" , &newnode->dod.day , &newnode->dod.month , &newnode->dod.year);

  //adding to the head of the  first linked list 
  newnode->next = s;
  s = newnode ;

  //adding to the head of the second linked list 

  Tlist *newnodeA = (Tlist*)malloc(sizeof(Tlist));
     *newnodeA = *newnode ;
      newnodeA->next = a;
      a = newnodeA;

  // Updating our file

  FILE *f = fopen("database.txt", "a");
  if(f != NULL){

    fprintf(f, "%s=%s=%s=%s\n", name , definition , dob , dod);
    fclose(f);
  }
return s;
}


Tlist* addEvents(Tlist *b , char *nameEvente , char *date){
   // Creating new node 
     Tlist *newnode = (Tlist*)malloc(sizeof(Tlist)); 
     strcpy(newnode->name , nameEvente);
     strcpy(newnode->definition , "Historical Event");

     sscanf(date , "%d/%d/%d" , &newnode->dob.day , &newnode->dob.month , &newnode->dob.year);

     newnode->dod.day = 0;
     newnode->dod.month =0;
     newnode->dod.year = 0;
 

   // adding at the head of the second linked list 

   newnode->next = b;
   b = newnode ;


   //Updating the text file

   FILE *f = fopen("database.txt", "a");
   if(f != NULL){

    fprintf(f, "%s=Historical Event=%s=00/00/0000\n", nameEvente , date);
    fclose(f);
   }
  return b;
}


int countWords(char *str){
    int count = 0;
    int i = 0;
    if(str[0] == '\0') return 0;

    while(str[i] != '\0'){
       if(str[i] == ' ' && str[i+1] != ' ' && str[i+1] != '\0'){
          count++;
       }
      i++;
    }
return count + 1;
}

//! TQueue* sName(TList *s)
TQueue* sName(Tlist *s){
  TQueue* q = (TQueue*)malloc(sizeof(TQueue));
  q->head = q->tail = NULL;

  if(s == NULL) return q;

  Tlist *curr;
  Tlist *tempS = s;

}
  
//! TQueue* ageP(TList *a)
//!
//!
//!
//!
//!
//!
//!
//!
//!
//!
//!


//! TQueue* toQueue(TList *merged)
TQueue* toQueue(Tlist *merged){

  TQueue* q = (TQueue*)malloc(sizeof(TQueue));
  q->head = NULL;
  q->tail = NULL;

  if(merged == NULL)  return q;


  Tlist *current = merged;
    // Note: If 'merged' came from merge2Nodes, it is circular. 
    // We need to handle the loop termination correctly.
  Tlist *startNode = merged ;

  do{
   QNode* newnode = (QNode*)malloc(sizeof(QNode));
   strcpy(newnode->name , current->name);
   strcpy(newnode->definition , current->definition);

    newnode->dob = current->dob;
    newnode->dod = current->dod;
    newnode->next = NULL;


    if(q->tail == NULL){
      q->head = q->tail = newnode;
    }else{
     q->tail->next = newnode;
     q->tail = newnode;
    }
    
    current = current->next;


  } while(current != NULL && current != startNode);

return q;
}



void enqueue(TQueue *q , char *name , char *definition , date dob , date dod){
   QNode *newNode = (QNode*)malloc(sizeof(QNode));
   strcpy(newNode->name , name);
   strcpy(newNode->definition , definition);

   newNode->dob = dob;
   newNode->dod = dod;
   newNode->next = NULL;

   if(q->tail == NULL){
    q->head = newNode;
    q->tail = newNode;

   }else{
    q->tail->next = newNode;
    q->tail = newNode;
   }

}

TBilist* insertBilist(TBilist *head, char *name, char *definition, date dob, date dod){
    TBilist *newNode = (TBilist*)malloc(sizeof(TBilist));
    strcpy(newNode->name ,name);
    strcpy(newNode->definition ,definition);

    newNode->dob = dob;
    newNode->dod = dod;
    newNode->next = NULL;
    newNode->prev = NULL;

    if(head == NULL){
      return newNode;

    }

    TBilist *current = head;
    while(current->next != NULL){
      current = current->next;

    }

     current->next = newNode;
     newNode->prev = current;

     return head;

}