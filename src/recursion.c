#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structs.h"
#include "stacks.h"



int countOccurence(Tlist *f , char *name){
    if (f == NULL) return 0 ;

    if(strcmp(f->name , name)== 0){
        return 1 + countOccurence(f->next,name);
    }

   return countOccurence(f->next,name);

}

Tlist* removeOccurence(Tlist *f , char *word){
  if(f == NULL) return NULL;

  if(strcmp(f->name , word) == 0){
    Tlist *next = f->next;
    free(f);
    return removeOccurence(next , word);
  }

  f->next = removeOccurence(f->next , word);
  return f;

}

Tlist* replaceOccurence(Tlist *f , char *name , char *DoB , char *DoD){
    if(f == NULL) return NULL;

    if(strcmp(f->name , name) == 0){
        f->dob = stringToDate(DoB);
        f->dod = stringToDate(DoD);

    }

    f->next = replaceOccurence(f->next , name , DoB , DoD);
    return f;
}

//! Helper Function 

void swap(char *a , char* b){
     char temp = *a;
     *a = *b;
     *b = temp;
}

void namePermutation(char *name , int start , int end){
    if(start == end){
        printf("%s\n", name);
        return;
    }

    int i;

    for(i =start ; i <= end ; i++){
        swap(&name[start], &name[i]);
        namePermutation(name , start + 1 , end);
        swap(&name[start] , &name[i]);
    }
}

//! to Revise !\\

void subseqName(char *word , char *current , int index){
    if(index == strlen(word)){
        printf("%s\n", current);
        return;
    }
    // exclude current character
    subseqName(word, current, index + 1);

    // include current character
    int len = strlen(current);
    current[len] = word[index];
    current[len + 1] = '\0';
    subseqName(word, current, index + 1);

    // backtrack
    current[len] = '\0';
}

void longestSubyear(Tlist *f , char *date1 , char *date2){
     if(f == NULL) return ;

     date d1 = stringToDate(date1);
     date d2 = stringToDate(date1);

     if(isOverlapping(f->dob , f->dod , d1 , d2)){
           printf("Name: %s | DoB: %02d/%02d/%04d | DoD: %02d/%02d/%04d\n",
            f->name,
            f->dob.day , f->dob.month , f->dob.year ,
            f->dod.day , f->dod.month , f->dod.year  );
           }      


        longestSubyear(f->next , date1 , date2);

     
}

//! to Revise !\\

int distinctSubseqWord(char *event) {
    if (strlen(event) == 0) return 1;

    int count = 2 * distinctSubseqWord(event + 1);

    // check for duplicate characters
    int i;
    for (i = 1; event[i] != '\0'; i++) {
        if (event[i] == event[0]) {
            count -= distinctSubseqWord(event + i + 1);
            break;
        }
    }

    return count;
}





bool isPalindromWord(char *event){

    char copy[100];
    strcpy(copy , event);

    int len = strlen(event);

    if(len == 0 || len == 1) return true;

    if(copy[0] != copy[len - 1]) return false;

    // removing the last char
    copy[len - 1] = '\0';

    return isPalindromWord(copy + 1);
}



