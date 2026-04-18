#ifndef RECURSION_H
#define RECURSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structs.h"
#include "stacks.h"

//! ─── RECURSION FUNCTIONS ────────

int countOccurence(Tlist *f, char *name);
Tlist* removeOccurence(Tlist *f, char *word);
Tlist* replaceOccurence(Tlist *f, char *name, char *DoB, char *DoD);

//! ─── PERMUTATION & SUBSEQUENCES ──────

void swap(char *a, char *b);
void namePermutation(char *name, int start, int end);
void subseqName(char *word, char *current, int index);
int distinctSubseqWord(char *event);

//! ─── DATE & PALINDROME ──────────

void longestSubyear(Tlist *f, char *date1, char *date2);
bool isPalindromeWord(char *event);

#endif