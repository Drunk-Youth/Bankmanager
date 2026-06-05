#include <stdio.h>
#include <stdlib.h>
#include "bank.h"
#include <io.h>

#define FILE_NAME bank.bat

//要操作的全局变量，但不在本模块定义需要声明为外部引入
extern STACCOUNT g_astAccounts[];
extern int g_iAccCount;

/* 函数功能：数据持久化，将内存中的账户数据保存到文件中
             保存方式为二进制
             保存到文件bank.bat中，该文件名只在本模块用使用
 * 参数：无
 *       保存信息来自全局变量g_astAccounts
 * 返回值：无
 *       保存是失败怎么处理？
 */
void SaveData()
{
    FILE *file = fopen("bank.dat", "wb");
    fwrite(g_astAccounts, sizeof(STACCOUNT), 50, file);
    fclose(file);

    FILE *logfile = fopen("log.dat", "wb");
    fwrite(&g_logQueue, sizeof(LogQueue), 1, logfile);
    fclose(logfile);

    LoadData();
}


/* 函数功能：从文件bank.bat中把账户数据读取到内存
             操作的文件名仅在本模块中使用
 * 参数：无
 *       信息读取后放入全局变量g_astAccounts
 * 返回值：无
 *        读取失败怎么处理？
 */
void LoadData()
{
    int i;
    int result=0;
    FILE *file = fopen("bank.dat", "rb");
    if (file == NULL) {
        InitAdminAccount();
        g_iAccCount= 1;
    }
    else {
        fread(g_astAccounts, sizeof(STACCOUNT), 50, file);
    }
    fclose( file);
    for (i = 0; i < 50; i++) {
        if (g_astAccounts[i].id != 0) {
            result++;
        }
    }
    g_iAccCount= result;

    InitLogQueue();
    FILE *logfile = fopen("log.dat", "rb");
    if (logfile != NULL) {
        fread(&g_logQueue, sizeof(LogQueue), 1, logfile);
        fclose(logfile);
    }
}

void SaveMerkleRoot() {
    FILE* f = fopen("merkle.dat", "wb");
    if (!f) return;
    fwrite(g_merkleRoot.data, 1, HASH_SIZE, f);
    fclose(f);
}

void LoadMerkleRoot() {
    FILE* f = fopen("merkle.dat", "rb");
    if (!f) {
        memset(g_merkleRoot.data, 0, HASH_SIZE);
        return;
    }
    fread(g_merkleRoot.data, 1, HASH_SIZE, f);
    fclose(f);
}
