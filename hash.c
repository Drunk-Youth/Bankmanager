#include <stdio.h>
#include "bank.h"

extern STACCOUNT g_astAccounts[];
extern int g_iAccCount;

HashEntry g_hashTable[HASH_SIZE];

static int HashFunc(int id)
{
    return id % HASH_SIZE;
}

void InitHashTable()
{
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        g_hashTable[i].id = -1;
        g_hashTable[i].index = -1;
    }
}

int HashInsert(int id, int index)
{
    int pos = HashFunc(id);
    int first_tombstone = -1;
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        int probe = (pos + i) % HASH_SIZE;
        if (g_hashTable[probe].id == id) {
            g_hashTable[probe].index = index;
            return 0;
        }
        if (g_hashTable[probe].id == -1) {
            if (first_tombstone != -1) {
                probe = first_tombstone;
            }
            g_hashTable[probe].id = id;
            g_hashTable[probe].index = index;
            return 0;
        }
        if (g_hashTable[probe].id == -2 && first_tombstone == -1) {
            first_tombstone = probe;
        }
    }
    return -1;
}

int HashSearch(int id)
{
    int pos = HashFunc(id);
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        int probe = (pos + i) % HASH_SIZE;
        if (g_hashTable[probe].id == id) {
            return g_hashTable[probe].index;
        }
        if (g_hashTable[probe].id == -1) {
            return -1;
        }
    }
    return -1;
}

void HashDelete(int id)
{
    int pos = HashFunc(id);
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        int probe = (pos + i) % HASH_SIZE;
        if (g_hashTable[probe].id == -1) {
            return;
        }
        if (g_hashTable[probe].id == id) {
            g_hashTable[probe].id = -2;
            g_hashTable[probe].index = -1;
            return;
        }
    }
}

void RebuildHashTable()
{
    int i;
    InitHashTable();
    for (i = 0; i < g_iAccCount; i++) {
        if (g_astAccounts[i].id != 0) {
            HashInsert(g_astAccounts[i].id, i);
        }
    }
}
