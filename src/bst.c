#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stacks.h"
#include <stdbool.h>

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


// helper : finds the smallest node in a subtree
TTree minNode(TTree root){
    while(root->left != NULL)
        root = root->left;
    return root;
}


TTree insertBST(TTree root , char *name , char *definition , date dob , date dod){

    TNode *newnode = (TNode*)malloc(sizeof(TNode));
    strcpy(newnode->name, name);
    strcpy(newnode->definition, definition);
    newnode->dob = dob;
    newnode->dod = dod;
    newnode->left = NULL;
    newnode->right = NULL;

    if(root == NULL)
        return newnode;

    if(strcmp(name , root->name) < 0)
        root->left = insertBST(root->left , name , definition , dob , dod);
    else
        root->right = insertBST(root->right , name , definition , dob , dod);

    return root;
}

// stk passed by value so original stack is not destroyed
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
    int dobYear , dodYear;
    date dob , dod;

    while(fgets(buffer , sizeof(buffer) , f) != NULL){
        if(sscanf(buffer , "%[^=]=%[^=]=%d=%d" , name , definition , &dobYear , &dodYear) == 4){
            dob.year = dobYear;
            dod.year = dodYear;
            root = insertBST(root , name , definition , dob , dod);
        }
    }
    return root;
}

TTree getInfoNameTree(TTree *tr , char *name){
    if(*tr == NULL)
        return NULL;

    if(strcmp(name , (*tr)->name) == 0)
        return *tr;

    if(strcmp(name , (*tr)->name) < 0)
        return getInfoNameTree(&(*tr)->left , name);

    return getInfoNameTree(&(*tr)->right , name);
}

TTree addNameBST(TTree *tr , char *name , char *definition , char *DoB , char *DoD){
    date dob , dod;
    dob.year = atoi(DoB);
    dod.year = atoi(DoD);

    *tr = insertBST(*tr , name , definition , dob , dod);
    return *tr;
}

TTree updateNameBST(TTree *tr , char *name , char *s , char *DoB , char *DoD){
    TTree node = getInfoNameTree(tr , name);
    if(node == NULL)
        return NULL;

    strcpy(node->definition , s);
    node->dob.year = atoi(DoB);
    node->dod.year = atoi(DoD);

    return *tr;
}

void traversalBSTinorder(TTree tr){
    if(tr == NULL)
        return;

    traversalBSTinorder(tr->left);

    printf("Name: %s\n" , tr->name);
    printf("Definition: %s\n" , tr->definition);
    printf("Born: %d\n" , tr->dob.year);
    printf("Died: %d\n" , tr->dod.year);

    traversalBSTinorder(tr->right);
}

void traversalBSTpreorder(TTree tr){
    if(tr == NULL)
        return;

    printf("Name: %s\n" , tr->name);
    printf("Definition: %s\n" , tr->definition);
    printf("Born: %d\n" , tr->dob.year);
    printf("Died: %d\n" , tr->dod.year);

    traversalBSTpreorder(tr->left);
    traversalBSTpreorder(tr->right);
}

void traversalBSTpostorder(TTree tr){
    if(tr == NULL)
        return;

    traversalBSTpostorder(tr->left);
    traversalBSTpostorder(tr->right);

    printf("Name: %s\n" , tr->name);
    printf("Definition: %s\n" , tr->definition);
    printf("Born: %d\n" , tr->dob.year);
    printf("Died: %d\n" , tr->dod.year);
}

int heightBST(TTree tr){
    if(tr == NULL)
        return 0;

    int leftH  = heightBST(tr->left);
    int rightH = heightBST(tr->right);

    if(leftH > rightH)
        return 1 + leftH;
    else
        return 1 + rightH;
}

int sizeBST(TTree tr){
    if(tr == NULL)
        return 0;
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
        return lowestCommonAncestor(&(*tr)->left , word1 , word2);

    if(strcmp(word1 , (*tr)->name) > 0 && strcmp(word2 , (*tr)->name) > 0)
        return lowestCommonAncestor(&(*tr)->right , word1 , word2);

    return *tr;
}

int countNodesRange(TTree tr , int l , int h){
    if(tr == NULL)
        return 0;

    if(tr->dob.year >= l && tr->dob.year <= h)
        return 1 + countNodesRange(tr->left , l , h) + countNodesRange(tr->right , l , h);

    if(tr->dob.year < l)
        return countNodesRange(tr->right , l , h);

    return countNodesRange(tr->left , l , h);
}

TTree inOrderSuccessor(TTree *tr , char *word){
    TTree successor = NULL;
    TTree current = *tr;

    while(current != NULL){
        if(strcmp(word , current->name) < 0){
            successor = current;
            current = current->left;
        }
        else if(strcmp(word , current->name) > 0){
            current = current->right;
        }
        else{
            if(current->right != NULL)
                return minNode(current->right);
            break;
        }
    }
    return successor;
}

TTree BSTMirror(TTree *tr){
    if(*tr == NULL)
        return NULL;

    TTree temp   = (*tr)->left;
    (*tr)->left  = (*tr)->right;
    (*tr)->right = temp;

    BSTMirror(&(*tr)->left);
    BSTMirror(&(*tr)->right);

    return *tr;
}

bool isBalancedBST(TTree tr){
    if(tr == NULL)
        return true;

    int leftH  = heightBST(tr->left);
    int rightH = heightBST(tr->right);

    int diff = leftH - rightH;
    if(diff < 0)
        diff = -diff;

    if(diff > 1)
        return false;

    return isBalancedBST(tr->left) && isBalancedBST(tr->right);
}

// helper : stores inorder traversal into an array
void storeInOrder(TTree tr , TNode **arr , int *i){
    if(tr == NULL)
        return;
    storeInOrder(tr->left , arr , i);
    arr[(*i)++] = tr;
    storeInOrder(tr->right , arr , i);
}

// helper : builds a balanced BST from a sorted array
TTree sortedArrayToBST(TNode **arr , int start , int end){
    if(start > end)
        return NULL;

    int mid = (start + end) / 2;
    TTree root = arr[mid];

    root->left  = sortedArrayToBST(arr , start , mid - 1);
    root->right = sortedArrayToBST(arr , mid + 1 , end);

    return root;
}

// helper : merges two sorted arrays into one
TNode **mergeSortedArrays(TNode **arr1 , int n1 , TNode **arr2 , int n2 , int *total){
    *total = n1 + n2;
    TNode **merged = (TNode**)malloc(*total * sizeof(TNode*));

    int i = 0 , j = 0 , k = 0;
    while(i < n1 && j < n2){
        if(strcmp(arr1[i]->name , arr2[j]->name) < 0)
            merged[k++] = arr1[i++];
        else
            merged[k++] = arr2[j++];
    }
    while(i < n1) merged[k++] = arr1[i++];
    while(j < n2) merged[k++] = arr2[j++];

    return merged;
}

TTree BTSMerge(TTree *tr1 , TTree *tr2){
    int n1 = sizeBST(*tr1);
    int n2 = sizeBST(*tr2);

    TNode **arr1 = (TNode**)malloc(n1 * sizeof(TNode*));
    TNode **arr2 = (TNode**)malloc(n2 * sizeof(TNode*));

    int i = 0 , j = 0;
    storeInOrder(*tr1 , arr1 , &i);
    storeInOrder(*tr2 , arr2 , &j);

    int total;
    TNode **merged = mergeSortedArrays(arr1 , n1 , arr2 , n2 , &total);

    TTree result = sortedArrayToBST(merged , 0 , total - 1);

    free(arr1);
    free(arr2);
    free(merged);

    return result;
}

TTree deleteNameBST(TTree *tr , char *name){
    if(*tr == NULL)
        return NULL;

    if(strcmp(name , (*tr)->name) < 0)
        (*tr)->left = deleteNameBST(&(*tr)->left , name);

    else if(strcmp(name , (*tr)->name) > 0)
        (*tr)->right = deleteNameBST(&(*tr)->right , name);

    else{
        // no left child
        if((*tr)->left == NULL){
            TNode *temp = *tr;
            *tr = (*tr)->right;
            free(temp);
        }
        // no right child
        else if((*tr)->right == NULL){
            TNode *temp = *tr;
            *tr = (*tr)->left;
            free(temp);
        }
        // two children
        else{
            TTree successor = minNode((*tr)->right);
            strcpy((*tr)->name , successor->name);
            strcpy((*tr)->definition , successor->definition);
            (*tr)->dob = successor->dob;
            (*tr)->dod = successor->dod;
            (*tr)->right = deleteNameBST(&(*tr)->right , successor->name);
        }
    }
    return *tr;
}
