#include <stdio.h>
#include "stacks.h"

typedef struct {
    int year;
} date;

typedef struct TNode {
    char name[100];
    char definition[250];
    date dob;
    date dod;
    struct TNode* left;
    struct TNode* right;
} TNode;

typedef TNode* TTree;

TTree insertBST(TTree root , char *name , char *definition , date dob ,date dod){

    TNode *newnode = (TNode*)malloc(sizeof(TNode));
    strcpy(newnode->definition, definition);
    strcpy(newnode->name, name);

    newnode->dob = dob;
    newnode->dod = dod;
    newnode->left = NULL;
    newnode->right = NULL;

    if(root == NULL){
        return newnode;
    }

    if(strcmp(name , root->name) < 0){
        root->left = insertBST(root->left , name , definition , dob ,dod);

    }else{
        root->right = insertBST(root->right , name ,definition , dob ,dod);

    }

    return root ;
}

//! here we passed stk by value not by pointer so the original
//! stack outside the function is not destroyed

TTree toTree(TStack stk){
    TTree root = NULL;
    Tlist *temp = NULL;

    while(stk.top != NULL){
        temp = pop(&stk);
        root = insertBST(root, 
                         temp->name, 
                         temp->definition, 
                         temp->dob, 
                         temp->dod);
        free(temp);

    }
    return root;

}

TTree fillTree(FILE *f){
    TTree root = NULL;
    char buffer[500];
    char name[100];
    char definition[250];
    int dobYear, dodYear;
    date dob, dod;


    while(fgets(buffer , sizeof(buffer) , f) != NULL){
        if(sscanf(buffer , "%[^=]=%[^=]=%d=%d", name, definition, &dobYear, &dodYear) == 4){
                        dob.year = dobYear;
                        dod.year = dodYear;
                        root = insertBST(root ,name ,definition ,dob ,dod);
        }

}
     return root;
}

TTree getInfoNameTree(TTree *tr , char *name){
    if(*tr == NULL){
        return NULL;
    }

    if(strcmp(name, (*tr)->name) == 0){
        return tr;
    }

    if(strcmp(name , (*tr)->name) < 0){
        return getInfoNameTree(&(*tr)->left  , name);
    }

    return getInfoNameTree(&(*tr)->right , name);

}

TTree addNameBST(TTree *tr , char *name , char *definition , char *DoB , char *DoD){
    date dob , dod ;
    dob.year = atoi(DoB);
    dob.year = atoi(DoD);

    *tr = insertBST(*tr , name , definition , dob , dod);

    return *tr;

}

//!  TTree deleteNameBST(TTree *tr , char *name)
//!
//!
//!
//!
//!
//!
//!
//!

TTree updateNameBST(TTree *tr , char *name , char *s ,char *DoB , char *DoD){

    TTree node = getInfoNameTree(tr , name);
    if(node == NULL){
        return NULL;
    }

    strcpy(node->definition , s);
    node->dob.year = atoi(DoB);
    node->dod.year = atoi(DoD);

    return *tr;
}

void traversalBSTinorder(TTree tr){

    if(tr == NULL){
        return;
    }

    traversalBSTinorder(tr->left);

    printf("Name: %s\n",  tr->name);
    printf("Definition: %s\n",  tr->definition);
    printf("Born: %s\n",  tr->dob.year);
    printf("Died: %s\n",  tr->dod.year);

    traversalBSTinorder(tr->right);
}

void traversalBSTpreorder(TTree tr){

    if(tr == NULL){
        return;
    }

    printf("Name: %s\n",  tr->name);
    printf("Definition: %s\n",  tr->definition);
    printf("Born: %s\n",  tr->dob.year);
    printf("Died: %s\n",  tr->dod.year);

    traversalBSTpreorder(tr->left);
    traversalBSTpreorder(tr->right);

}

void traversalBSTpostorder(TTree tr){

    if(tr == NULL){
        return;
    }

    traversalBSTpostorder(tr->left);
    traversalBSTpostorder(tr->right);

    printf("Name: %s\n",  tr->name);
    printf("Definition: %s\n",  tr->definition);
    printf("Born: %s\n",  tr->dob.year);
    printf("Died: %s\n",  tr->dod.year);

}

int heightBST(TTree tr){
    if(tr == NULL)
        return NULL;
    

    int leftH = heightBST(tr->left);
    int rightH = heightBST(tr->right);

    if(leftH > rightH)
        return 1 + leftH;
    else
        return 1 + rightH;

    
}

int sizeBST(TTree tr){
    if(tr == NULL)
         return NULL;

        return 1 + sizeBST(tr->left) + sizeBST(tr->right);

}

void heightSizeBST(TTree tr){

    printf("Height : %d\n" , heightBST(tr));
    printf("Size : %d\n" , sizeBST(tr));

}

TTree lowestCommonAncestor(TTree *tr , char *word1 , char *word2){

    if(*tr == NULL)
       return NULL;

    if(strcmp(word1 , (*tr)->name) < 0 && strcmp(word2 , (*tr)->name) < 0)
       return lowestCommonAncestor((*tr)->left , word1 , word2);

    if(strcmp(word1 , (*tr)->name) > 0 && strcmp(word2 , (*tr)->name) > 0)
       return lowestCommonAncestor((*tr)->right , word1 , word2);


    return *tr;

}

int countNodesRange(TTree tr , int l , int h){

    if(tr == NULL)
       return NULL;

    if(tr->dob.year >= l && tr->dob.year <= h)
       return 1 + countNodesRange(tr->left , l , h) + countNodesRange(tr->right , l ,h);
    
    if(tr->dob.year < 1)
      return countNodesRange(tr->right , l , h);

    return countNodesRange(tr->left , l ,h);
}

//! TTree* inOrderSuccessor(TTree *tr, char *word
//! TTree* BSTMirror(TTree *tr)
//! bool isBalancedBST(TTree *tr)
//! TTree* BTSMerge(TTree *tr1, TTree *tr2)