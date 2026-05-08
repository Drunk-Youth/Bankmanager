#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bank.h"

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
    strcpy(g_logQueue.logs[pos].location, "四川省 德阳市"); // 根据你的需求固定地点
    
    // 队尾指针后移
    g_logQueue.rear = (g_logQueue.rear + 1) % MAX_LOGS;
    
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