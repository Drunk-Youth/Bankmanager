#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <wincrypt.h>
#include "bank.h"

HashValue g_merkleRoot;

static HCRYPTPROV g_hProv = 0;

static void ensureProvider() {
    if (g_hProv == 0)
        CryptAcquireContext(&g_hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
}

static void sha256(unsigned char* out, const unsigned char* data1, unsigned int len1,
                                      const unsigned char* data2, unsigned int len2) {
    HCRYPTHASH hHash = 0;
    DWORD hashLen = HASH_SIZE;
    ensureProvider();
    CryptCreateHash(g_hProv, CALG_SHA_256, 0, 0, &hHash);
    if (data1 && len1) CryptHashData(hHash, data1, len1, 0);
    if (data2 && len2) CryptHashData(hHash, data2, len2, 0);
    CryptGetHashParam(hHash, HP_HASHVAL, out, &hashLen, 0);
    CryptDestroyHash(hHash);
}

/* ==================== Merkle Tree ==================== */

void HashLogEntry(const LogEntry* entry, HashValue* out) {
    char buf[256];
    sprintf(buf, "ID:%d Type:%s Amount:%.2f Balance:%.2f Time:%s Loc:%s",
            entry->account_id, entry->type, entry->amount,
            entry->balance, entry->timestamp, entry->location);

    sha256(out->data, (unsigned char*)buf, (unsigned int)strlen(buf), NULL, 0);
}

static void hashPair(const HashValue* a, const HashValue* b, HashValue* out) {
    sha256(out->data, a->data, HASH_SIZE, b->data, HASH_SIZE);
}

void BuildMerkleTree(HashValue* root) {
    int count = GetLogCount();

    if (count == 0) {
        memset(root->data, 0, HASH_SIZE);
        return;
    }

    HashValue* leaves = (HashValue*)malloc(count * sizeof(HashValue));
    if (!leaves) return;

    int i = g_logQueue.front;
    int idx = 0;
    while (i != g_logQueue.rear) {
        HashLogEntry(&g_logQueue.logs[i], &leaves[idx]);
        i = (i + 1) % MAX_LOGS;
        idx++;
    }

    int n = count;
    while (n > 1) {
        int j;
        for (j = 0; j < n / 2; j++)
            hashPair(&leaves[2 * j], &leaves[2 * j + 1], &leaves[j]);

        if (n % 2 == 1) {
            hashPair(&leaves[n - 1], &leaves[n - 1], &leaves[n / 2]);
            n = n / 2 + 1;
        } else {
            n = n / 2;
        }
    }

    memcpy(root->data, leaves[0].data, HASH_SIZE);
    free(leaves);
}

void HashToHex(const HashValue* hash, char* hex_out) {
    int i;
    for (i = 0; i < HASH_SIZE; i++)
        sprintf(hex_out + i * 2, "%02x", hash->data[i]);
    hex_out[HASH_SIZE * 2] = '\0';
}

void PrintMerkleRoot() {
    char hex[65];
    HashToHex(&g_merkleRoot, hex);
    printf("\n当前默克尔根: %s\n", hex);
}

int VerifyMerkleTree() {
    HashValue computed;
    BuildMerkleTree(&computed);
    return memcmp(computed.data, g_merkleRoot.data, HASH_SIZE) == 0;
}