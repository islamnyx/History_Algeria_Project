#ifndef BST_H
#define BST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

// helper functions
TTree minNode(TTree root);
TTree insertBST(TTree root , char *name , char *definition , date dob , date dod);
void  storeInOrder(TTree tr , TNode **arr , int *i);
TTree sortedArrayToBST(TNode **arr , int start , int end);
TNode **mergeSortedArrays(TNode **arr1 , int n1 , TNode **arr2 , int n2 , int *total);
int   heightBST(TTree tr);
int   sizeBST(TTree tr);

// main functions
TTree toTree(TStack stk);
TTree fillTree(FILE *f);
TTree getInfoNameTree(TTree *tr , char *name);
TTree addNameBST(TTree *tr , char *name , char *definition , char *DoB , char *DoD);
TTree deleteNameBST(TTree *tr , char *name);
TTree updateNameBST(TTree *tr , char *name , char *s , char *DoB , char *DoD);
void  traversalBSTinorder(TTree tr);
void  traversalBSTpreorder(TTree tr);
void  traversalBSTpostorder(TTree tr);
void  heightSizeBST(TTree tr);
TTree lowestCommonAncestor(TTree *tr , char *word1 , char *word2);
int   countNodesRange(TTree tr , int l , int h);
TTree inOrderSuccessor(TTree *tr , char *word);
TTree BSTMirror(TTree *tr);
bool  isBalancedBST(TTree tr);
TTree BTSMerge(TTree *tr1 , TTree *tr2);

#endif
