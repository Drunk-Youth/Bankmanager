#include <stdio.h>
#include <stdlib.h>
#include "bank.h"

extern STACCOUNT g_astAccounts[];
extern int g_iAccCount;

/* 函数功能：处理用户的存款操作
             需要什么步骤？
 * 参数：无
 *       存款信息要写入全局变量g_astAccounts中
 * 返回值：无
 *        存款失败怎么处理？
 */
void Deposit(int id)
{
    int amount;
    int temp_password;
    while (1) {
        printf("请输入存款金额");
        scanf("%d",&amount);
        if (amount>0) {
            break;
        }
        printf("？你存给我存个负的？重新说存多少");
    }
    printf("请输入密码");
    while (1) {
        scanf("%d",&temp_password);
        if (g_astAccounts[FindAccount(id)].password==temp_password) {
            g_astAccounts[FindAccount(id)].balance+=amount;
            printf("success");
            QueryBalance(id);
            EnqueueLog(id, "Deposit", amount, g_astAccounts[FindAccount(id)].balance, -1);
            SaveData();
            break;
        }
            else
            {
                {
                    printf("密码错误，请重新输入");
                }
            }
        }
    }

/* 函数功能：处理用户的取款操作
             需要什么步骤？
 * 参数：无
 *       取款后，账户信息要写入全局变量g_astAccounts中
 * 返回值：无
 *       取款失败怎么处理？
 */
void Withdraw(int id)
{
    int amount;
    int temp_password;
    while (1) {
        printf("请输入取款金额");
        scanf("%d",&amount);
        if (amount>0) {
            break;
        }
        printf("？取款怎么能取负的呢");
    }
    while (1) {
        printf("请输入密码");
        scanf("%d",&temp_password);
        if (g_astAccounts[FindAccount(id)].password==temp_password) {
            if (g_astAccounts[FindAccount(id)].balance>amount) {
                g_astAccounts[FindAccount(id)].balance-=amount;
                EnqueueLog(id, "Withdraw", amount, g_astAccounts[FindAccount(id)].balance, -1);
                printf("success");
                QueryBalance(id);
                SaveData();
                return;
            }
            else {
                printf("有那么多钱吗你就取？\n");
                QueryBalance(id);
                return;
            }
        }
            else {
                printf("密码错误，请重新输入");
            }
        }

}

/* 函数功能：处理用户查询余额操作
             需要什么步骤？
 * 参数：无
 *       查询后全局变量是否需要变化？
 * 返回值：无
 *       查询失败怎么处理？
 */
void QueryBalance(int id)
{
    printf("你现在还有%f块\n",g_astAccounts[FindAccount(id)].balance);
    return;
}

void Transfer(int from_id)
{
    int to_id;
    int amount;
    int temp_password;
    int from_idx = FindAccount(from_id);
    int to_idx;

    printf("请输入目标账户ID: ");
    scanf("%d", &to_id);

    to_idx = FindAccount(to_id);
    if (to_idx == -1) {
        printf("目标账户不存在!\n");
        return;
    }
    if (from_id == to_id) {
        printf("不能给自己转账!\n");
        return;
    }

    while (1) {
        printf("请输入转账金额: ");
        scanf("%d", &amount);
        if (amount > 0) break;
        printf("金额必须为正数!\n");
    }

    printf("请输入密码验证身份: ");
    while (1) {
        scanf("%d", &temp_password);
        if (g_astAccounts[from_idx].password == temp_password) {
            if (g_astAccounts[from_idx].balance >= amount) {
                g_astAccounts[from_idx].balance -= amount;
                g_astAccounts[to_idx].balance += amount;
                EnqueueLog(from_id, "Transfer", amount,
                           g_astAccounts[from_idx].balance, to_id);
                EnqueueLog(to_id, "Receive", amount,
                           g_astAccounts[to_idx].balance, from_id);
                printf("转账成功! %d -> %d, 金额: %d\n",
                       from_id, to_id, amount);
                printf("当前余额: %.2f\n", g_astAccounts[from_idx].balance);
                SaveData();
                return;
            } else {
                printf("余额不足! 当前余额: %.2f\n",
                       g_astAccounts[from_idx].balance);
                return;
            }
        } else {
            printf("密码错误，请重新输入: ");
        }
    }
}
