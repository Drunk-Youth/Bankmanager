#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bank.h"

int GetLogCount() {
    if (g_logQueue.rear >= g_logQueue.front)
        return g_logQueue.rear - g_logQueue.front;
    else
        return MAX_LOGS - g_logQueue.front + g_logQueue.rear;
}

// 初始化日志队列
void InitLogQueue() {
    g_logQueue.front = 0;
    g_logQueue.rear = 0;
}

// 检查队列是否满
int IsLogQueueFull() {
    return (g_logQueue.rear + 1) % MAX_LOGS == g_logQueue.front;
}

// 检查队列是否空
int IsLogQueueEmpty() {
    return g_logQueue.front == g_logQueue.rear;
}

// 获取当前时间字符串
void GetCurrentTimeStr(char* buf) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// 入队操作：记录交易
// 参数: 账户ID, 类型("Deposit"/"Withdraw"), 交易金额, 交易后余额
int EnqueueLog(int id, const char* type, double amt, double bal) {
    // 如果队列满，通过移动 front 指针覆盖最旧的日志 (环形队列特性)
    if (IsLogQueueFull()) {
        // 覆盖旧数据，队头自动出队
        g_logQueue.front = (g_logQueue.front + 1) % MAX_LOGS;
    }
    
    // 准备日志条目
    int pos = g_logQueue.rear;
    g_logQueue.logs[pos].account_id = id;
    strcpy(g_logQueue.logs[pos].type, type);
    g_logQueue.logs[pos].amount = amt;
    g_logQueue.logs[pos].balance = bal;
    GetCurrentTimeStr(g_logQueue.logs[pos].timestamp);
    strcpy(g_logQueue.logs[pos].location, "宇宙总行"); // 根据你的需求固定地点
    
    // 队尾指针后移
    g_logQueue.rear = (g_logQueue.rear + 1) % MAX_LOGS;
    
    BuildMerkleTree(&g_merkleRoot);
    SaveMerkleRoot();
    
    return 1; // 成功
}

// 显示所有日志 (从队头到队尾)
void ShowTransactionLogs() {
    if (IsLogQueueEmpty()) {
        printf("暂无交易日志记录。\n");
        return;
    }
    
    printf("\n=== 交易流水日志 ===\n");
    printf("%-5s %-8s %-10s %-10s %-15s %-20s\n", "ID", "操作", "金额", "余额", "时间", "地点");
    printf("--------------------------------------------------------------\n");
    
    int i = g_logQueue.front;
    while (i != g_logQueue.rear) {
        LogEntry e = g_logQueue.logs[i];
        printf("%-5d %-8s %-10.2f %-10.2f %-15s %-20s\n", 
               e.account_id, e.type, e.amount, e.balance, e.timestamp, e.location);
        i = (i + 1) % MAX_LOGS;
    }
}

// 1. 构建部分匹配表 (Next Array)
// 这是KMP算法的核心预处理步骤
void ComputeLPSArray(const char* pattern, int M, int* lps) {
    int len = 0; // length of the previous longest prefix suffix
    lps[0] = 0; // lps[0] is always 0
    int i = 1;

    while (i < M) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

// 2. KMP 搜索函数
// 在文本 text 中查找模式 pattern，找到后调用回调函数处理匹配位置
void KMPSearch(const char* pattern, const char* text, void (*callback)(int)) {
    int M = strlen(pattern);
    int N = strlen(text);

    if (M == 0) return;

    // 创建并计算LPS数组
    int* lps = (int*)malloc(M * sizeof(int));
    ComputeLPSArray(pattern, M, lps);

    int i = 0; // index for text
    int j = 0; // index for pattern

    while (i < N) {
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }

        if (j == M) {
            // 找到匹配，调用回调函数输出该行或处理
            callback(i - j);
            j = lps[j - 1];
        } else if (i < N && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }

    free(lps);
}

// 3. 回调函数：用于打印匹配到的日志行号
void PrintMatchLine(int pos) {
    // 这里简化处理，实际应用中需要根据换行符计算行号
    // 或者直接打印匹配位置附近的上下文
    printf("找到匹配 (位置: %d)\n", pos);
}

// 4. 对外接口：搜索交易日志
void SearchLogs(const char* pattern) {
    if (IsLogQueueEmpty()) {
        printf("暂无日志可供搜索。\n");
        return;
    }

    printf("\n=== 搜索日志: '%s' ===\n", pattern);
    
    // 简化版：遍历每一条日志进行匹配
    int i = g_logQueue.front;
    int lineNum = 1;
    
    while (i != g_logQueue.rear) {
        LogEntry e = g_logQueue.logs[i];
        
        // 将日志条目格式化为字符串（模拟一行文本）
        char logLine[200];
        sprintf(logLine, "ID:%d Type:%s Amount:%.2f Balance:%.2f Time:%s", 
                e.account_id, e.type, e.amount, e.balance, e.timestamp);
        
        // 使用KMP检查这一行是否包含模式
        // 这里为了演示调用了标准 strstr，你可以将其替换为上面的 KMPSearch
        // 但考虑到单行文本较短，KMP优势不大；如果是超长日志文件，KMP优势巨大
        
        if (strstr(logLine, pattern) != NULL) {
            printf("%-3d %-8d %-10s %-10.2f %-15s\n", 
                   lineNum, e.account_id, e.type, e.amount, e.timestamp);
        }
        
        i = (i + 1) % MAX_LOGS;
        lineNum++;
    }
}