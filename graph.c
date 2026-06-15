#include <stdio.h>
#include <string.h>
#include "bank.h"

extern LogQueue g_logQueue;
extern STACCOUNT g_astAccounts[];
extern int g_iAccCount;

// 从转账日志构建邻接矩阵，matrix[i][j] = 账户i向账户j的累计转账金额
static void BuildAdjMatrix(int matrix[MAX_ACCOUNTS][MAX_ACCOUNTS])
{
    int i, j;
    for (i = 0; i < g_iAccCount; i++)
        for (j = 0; j < g_iAccCount; j++)
            matrix[i][j] = 0;

    if (IsLogQueueEmpty()) return;

    int idx = g_logQueue.front;
    while (idx != g_logQueue.rear) {
        LogEntry e = g_logQueue.logs[idx];
        if (strcmp(e.type, "Transfer") == 0 && e.target_id > 0) {
            int from_idx = FindAccount(e.account_id);
            int to_idx = FindAccount(e.target_id);
            if (from_idx != -1 && to_idx != -1) {
                matrix[from_idx][to_idx] += (int)e.amount;
            }
        }
        idx = (idx + 1) % MAX_LOGS;
    }
}

// 显示全部转账关系图
void ShowTransferGraph()
{
    int matrix[MAX_ACCOUNTS][MAX_ACCOUNTS];
    BuildAdjMatrix(matrix);

    printf("\n=== 客户资金关系图 ===\n");
    printf("(连线表示转账关系，数字为累计转账金额)\n\n");

    int has_transfer = 0;
    int i, j;
    for (i = 0; i < g_iAccCount; i++) {
        for (j = 0; j < g_iAccCount; j++) {
            if (matrix[i][j] > 0) {
                has_transfer = 1;
                printf("%s(%d) --[%d元]--> %s(%d)\n",
                       g_astAccounts[i].name, g_astAccounts[i].id,
                       matrix[i][j],
                       g_astAccounts[j].name, g_astAccounts[j].id);
            }
        }
    }

    if (!has_transfer) {
        printf("暂无转账记录，无法构建关系图。\n");
    }
}

// 查看指定账户的关联关系
void ShowAccountRelations(int id)
{
    int idx = FindAccount(id);
    if (idx == -1) {
        printf("账户不存在!\n");
        return;
    }

    int matrix[MAX_ACCOUNTS][MAX_ACCOUNTS];
    BuildAdjMatrix(matrix);

    printf("\n=== 账户关系分析: %s(%d) ===\n",
           g_astAccounts[idx].name, g_astAccounts[idx].id);

    int j;
    int out_count = 0;
    int out_total = 0;
    printf("\n[转出记录]\n");
    for (j = 0; j < g_iAccCount; j++) {
        if (matrix[idx][j] > 0) {
            printf("  -> %s(%d): %d元\n",
                   g_astAccounts[j].name, g_astAccounts[j].id, matrix[idx][j]);
            out_total += matrix[idx][j];
            out_count++;
        }
    }
    if (out_count == 0) printf("  无转出记录\n");
    else printf("  共转出 %d 笔, 合计 %d 元\n", out_count, out_total);

    int i;
    int in_count = 0;
    int in_total = 0;
    printf("\n[转入记录]\n");
    for (i = 0; i < g_iAccCount; i++) {
        if (matrix[i][idx] > 0) {
            printf("  <- %s(%d): %d元\n",
                   g_astAccounts[i].name, g_astAccounts[i].id, matrix[i][idx]);
            in_total += matrix[i][idx];
            in_count++;
        }
    }
    if (in_count == 0) printf("  无转入记录\n");
    else printf("  共转入 %d 笔, 合计 %d 元\n", in_count, in_total);

    printf("\n总结: 转出%d笔(共%d元), 转入%d笔(共%d元)\n",
           out_count, out_total, in_count, in_total);
}

// 查看最活跃账户排名（按转账关系数量排序）
void ShowMostActiveNodes()
{
    int matrix[MAX_ACCOUNTS][MAX_ACCOUNTS];
    BuildAdjMatrix(matrix);

    int degrees[MAX_ACCOUNTS];
    int account_ids[MAX_ACCOUNTS];
    int i, j;
    int count = 0;

    for (i = 0; i < g_iAccCount; i++) {
        int deg = 0;
        for (j = 0; j < g_iAccCount; j++) {
            if (matrix[i][j] > 0) deg++;
            if (matrix[j][i] > 0) deg++;
        }
        if (deg > 0) {
            degrees[count] = deg;
            account_ids[count] = i;
            count++;
        }
    }

    if (count == 0) {
        printf("\n暂无转账关系数据。\n");
        return;
    }

    // 冒泡排序（最多50个元素）
    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (degrees[j] < degrees[j + 1]) {
                int tmp_d = degrees[j];
                degrees[j] = degrees[j + 1];
                degrees[j + 1] = tmp_d;
                int tmp_id = account_ids[j];
                account_ids[j] = account_ids[j + 1];
                account_ids[j + 1] = tmp_id;
            }
        }
    }

    printf("\n=== 最活跃账户排名 ===\n");
    printf("%-4s %-8s %-20s %-10s\n", "排名", "ID", "姓名", "关系数");
    printf("-----------------------------------------\n");
    int top = count < 10 ? count : 10;
    for (i = 0; i < top; i++) {
        int idx = account_ids[i];
        printf("%-4d %-8d %-20s %-10d\n",
               i + 1, g_astAccounts[idx].id,
               g_astAccounts[idx].name, degrees[i]);
    }
}

// 用BFS查找两个账户间的最短转账路径
void FindTransferPaths(int from_id, int to_id)
{
    int from_idx = FindAccount(from_id);
    int to_idx = FindAccount(to_id);

    if (from_idx == -1) { printf("源账户不存在!\n"); return; }
    if (to_idx == -1) { printf("目标账户不存在!\n"); return; }
    if (from_id == to_id) { printf("同一账户无需查找路径。\n"); return; }

    int matrix[MAX_ACCOUNTS][MAX_ACCOUNTS];
    BuildAdjMatrix(matrix);

    // BFS
    int visited[MAX_ACCOUNTS] = {0};
    int parent[MAX_ACCOUNTS];
    int queue[MAX_ACCOUNTS];
    int q_front = 0, q_rear = 0;
    int i;

    for (i = 0; i < MAX_ACCOUNTS; i++) parent[i] = -1;

    queue[q_rear++] = from_idx;
    visited[from_idx] = 1;

    int found = 0;
    while (q_front < q_rear) {
        int curr = queue[q_front++];
        if (curr == to_idx) { found = 1; break; }
        int j;
        for (j = 0; j < g_iAccCount; j++) {
            if (matrix[curr][j] > 0 && !visited[j]) {
                visited[j] = 1;
                parent[j] = curr;
                queue[q_rear++] = j;
            }
        }
    }

    if (!found) {
        printf("未找到从 %d 到 %d 的转账路径。\n", from_id, to_id);
        return;
    }

    // 回溯路径
    int path[MAX_ACCOUNTS];
    int path_len = 0;
    int curr = to_idx;
    while (curr != -1) {
        path[path_len++] = curr;
        curr = parent[curr];
    }

    printf("\n=== 转账路径: %s(%d) -> %s(%d) ===\n",
           g_astAccounts[from_idx].name, from_id,
           g_astAccounts[to_idx].name, to_id);
    printf("最短路径长度: %d 步\n", path_len - 1);

    for (i = path_len - 1; i >= 0; i--) {
        printf("%s(%d)", g_astAccounts[path[i]].name, g_astAccounts[path[i]].id);
        if (i > 0) {
            printf(" --[%d元]--> ", matrix[path[i]][path[i - 1]]);
        }
    }
    printf("\n");
}

// 生成虚拟交易记录，用于测试资金关系图分析
void GenerateDummyData()
{
    int i;
    int orig_count = g_iAccCount;

    // 如果账户不足5个，先创建测试账户（密码统一123456）
    if (g_iAccCount < 5 && g_iAccCount < MAX_ACCOUNTS) {
        int target = 5;
        if (target > MAX_ACCOUNTS) target = MAX_ACCOUNTS;

        for (i = g_iAccCount; i < target; i++) {
            g_astAccounts[i].id = 1000 + i;
            g_astAccounts[i].balance = (i + 1) * 5000.0;
            g_astAccounts[i].password = 123456;
            g_astAccounts[i].quanxian = 2;
            g_astAccounts[i].name[0] = '\0';
        }

        // 给账户起中文名
        if (orig_count <= 1) strcpy(g_astAccounts[1].name, "张三");
        if (orig_count <= 2) strcpy(g_astAccounts[2].name, "李四");
        if (orig_count <= 3) strcpy(g_astAccounts[3].name, "王五");
        if (orig_count <= 4) strcpy(g_astAccounts[4].name, "赵六");

        g_iAccCount = target;
        SaveData();
        printf("已补充创建%d个测试账户\n", target - orig_count);
    }

    // 如果有第6个槽位，创建一个钱七
    if (g_iAccCount < 6 && g_iAccCount < MAX_ACCOUNTS) {
        i = g_iAccCount;
        g_astAccounts[i].id = 1000 + i;
        strcpy(g_astAccounts[i].name, "钱七");
        g_astAccounts[i].balance = 20000.0;
        g_astAccounts[i].password = 123456;
        g_astAccounts[i].quanxian = 2;
        g_iAccCount++;
        SaveData();
        printf("已创建测试账户: 钱七(1005)\n");
    }

    // 清空旧日志，生成新的虚拟交易流水
    InitLogQueue();

    int id[6];
    for (i = 0; i < g_iAccCount && i < 6; i++)
        id[i] = g_astAccounts[i].id;

    // 为每个账户存入初始资金（留痕）
    for (i = 1; i < g_iAccCount && i < 6; i++) {
        double bal = g_astAccounts[i].balance;
        EnqueueLog(id[i], "Deposit", bal, bal, -1);
    }

    // 构建一个互联的转账网络：
    // 张三 -> 李四 3000
    // 李四 -> 王五 2000
    // 王五 -> 赵六 1500
    // 赵六 -> 钱七 1000
    // 钱七 -> 张三 800   (形成闭环，可测路径查找)
    // 张三 -> 王五 1200  (增加多条边)
    // 李四 -> 钱七 600

    if (g_iAccCount >= 2) {
        // 张三(1) -> 李四(2): 3000
        g_astAccounts[1].balance -= 3000;
        g_astAccounts[2].balance += 3000;
        EnqueueLog(id[1], "Transfer", 3000, g_astAccounts[1].balance, id[2]);
        EnqueueLog(id[2], "Receive", 3000, g_astAccounts[2].balance, id[1]);
    }
    if (g_iAccCount >= 3) {
        // 李四(2) -> 王五(3): 2000
        g_astAccounts[2].balance -= 2000;
        g_astAccounts[3].balance += 2000;
        EnqueueLog(id[2], "Transfer", 2000, g_astAccounts[2].balance, id[3]);
        EnqueueLog(id[3], "Receive", 2000, g_astAccounts[3].balance, id[2]);
        // 张三(1) -> 王五(3): 1200
        g_astAccounts[1].balance -= 1200;
        g_astAccounts[3].balance += 1200;
        EnqueueLog(id[1], "Transfer", 1200, g_astAccounts[1].balance, id[3]);
        EnqueueLog(id[3], "Receive", 1200, g_astAccounts[3].balance, id[1]);
    }
    if (g_iAccCount >= 4) {
        // 王五(3) -> 赵六(4): 1500
        g_astAccounts[3].balance -= 1500;
        g_astAccounts[4].balance += 1500;
        EnqueueLog(id[3], "Transfer", 1500, g_astAccounts[3].balance, id[4]);
        EnqueueLog(id[4], "Receive", 1500, g_astAccounts[4].balance, id[3]);
    }
    if (g_iAccCount >= 5) {
        // 赵六(4) -> 钱七(5): 1000
        g_astAccounts[4].balance -= 1000;
        g_astAccounts[5].balance += 1000;
        EnqueueLog(id[4], "Transfer", 1000, g_astAccounts[4].balance, id[5]);
        EnqueueLog(id[5], "Receive", 1000, g_astAccounts[5].balance, id[4]);
        // 钱七(5) -> 张三(1): 800 (闭环)
        g_astAccounts[5].balance -= 800;
        g_astAccounts[1].balance += 800;
        EnqueueLog(id[5], "Transfer", 800, g_astAccounts[5].balance, id[1]);
        EnqueueLog(id[1], "Receive", 800, g_astAccounts[1].balance, id[5]);
        // 李四(2) -> 钱七(5): 600
        g_astAccounts[2].balance -= 600;
        g_astAccounts[5].balance += 600;
        EnqueueLog(id[2], "Transfer", 600, g_astAccounts[2].balance, id[5]);
        EnqueueLog(id[5], "Receive", 600, g_astAccounts[5].balance, id[2]);
    }

    // 为部分账户生成取款记录
    if (g_iAccCount >= 3) {
        g_astAccounts[1].balance -= 500;
        EnqueueLog(id[1], "Withdraw", 500, g_astAccounts[1].balance, -1);
        g_astAccounts[3].balance -= 300;
        EnqueueLog(id[3], "Withdraw", 300, g_astAccounts[3].balance, -1);
    }

    SaveData();
    printf("已生成 %d 条虚拟交易记录\n",
           (g_logQueue.rear - g_logQueue.front + MAX_LOGS) % MAX_LOGS);
    printf("账户数量: %d\n", g_iAccCount);
    printf("转账网络已构建: 张三->李四->王五->赵六->钱七->张三(闭环)\n");
}
