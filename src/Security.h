#ifndef SECURITY_H
#define SECURITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIG_FILE ".metadata.sig"
#define DB_FILE "database.txt"

// FNV-1a constants for 64-bit hashing
#define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL

/**
 * Calculates a unique hash for the database file.
 * If the file is modified by even one character, the hash changes completely.
 */
unsigned long long calculate_db_hash() {
    FILE *f = fopen(DB_FILE, "rb");
    if (!f) return 0;

    unsigned long long hash = FNV_OFFSET_BASIS;
    int c;

    while ((c = fgetc(f)) != EOF) {
        hash ^= (unsigned char)c;
        hash *= FNV_PRIME;
    }

    fclose(f);
    return hash;
}

/**
 * Saves the current file hash to a hidden signature file.
 * Call this every time you write to the database.
 */
void update_integrity_signature() {
    unsigned long long current_hash = calculate_db_hash();
    FILE *f = fopen(SIG_FILE, "w");
    if (f) {
        fprintf(f, "%llu", current_hash);
        fclose(f);
    }
}

/**
 * Checks if the database has been tampered with.
 * Returns: 1 if OK, 0 if TAMPERED, -1 if Signature Missing.
 */
int verify_database_integrity() {
    FILE *f = fopen(SIG_FILE, "r");
    if (!f) return -1; // First run, no signature yet

    unsigned long long stored_hash;
    if (fscanf(f, "%llu", &stored_hash) != 1) {
        fclose(f);
        return 0; // Signature file corrupted
    }
    fclose(f);

    unsigned long long actual_hash = calculate_db_hash();
    return (stored_hash == actual_hash);
}

#endif