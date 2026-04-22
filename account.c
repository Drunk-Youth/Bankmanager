#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bank.h"

extern STACCOUNT g_astAccounts[];
extern int g_iAccCount;

/* 函数功能：处理用户的创建账户操作
             思考处理步骤，写到函数体内注释
 * 参数：无
 *       存款信息要写入全局变量g_astAccounts中
 * 返回值：无
 *        存款失败怎么处理？
 */
void InitAdminAccount() {
    printf("初始化默认管理员账户");
    g_astAccounts[0].id = 1000;
    strcpy(g_astAccounts[0].name, "admin");
    g_astAccounts[0].balance = 0.0;
    g_astAccounts[0].password = 123456;
    g_astAccounts[0].quanxian = 1;
    SaveData();
}

void CreateAccount(int quanxian)
{
    // (1)合法性检查
    if(MAX_ACCOUNTS < g_iAccCount) 
    {
        printf("账户数量已达上限!\n");
        return;
    }
    
    // (2)定义局部变量（函数内使用的变量）
    STACCOUNT stNewAcc;     //账户变量，结构体变量定义
    //char cInput[NAME_LEN];  //接输入的名字字符串
    
    // (3)功能实现
    printf("请输入姓名(最多30个字符): ");
    
    //这里有风险，怎么改进？
    scanf("%30s", stNewAcc.name);
    //stNewAcc.name[NAME_LEN - 1] = '\0'; // 确保null终止
    //输密码
    int temp_password;
    while (1) {
        printf("请输入密码\n");
        scanf("%d",&temp_password);
        if (99999<temp_password&&temp_password<999999) {
            printf("设置成功\n");
            stNewAcc.password=temp_password;
            SaveData();
            break;
        }
        printf("密码不符合规范！请重新输入\n");
    }
    //需求中要求账户ID从1000开始，初始化账户余额
    stNewAcc.id = 1000 + g_iAccCount;
    stNewAcc.balance = 0.0;
    stNewAcc.quanxian = quanxian;
    g_astAccounts[g_iAccCount] = stNewAcc;
    
    //把记录账户数量的全局变量自增。加过之后自增，所以下标0占用，数量是1个
    g_iAccCount++;
    
    printf("开户成功! 账户ID: %d\n", stNewAcc.id);
    SaveData();
}

/* 函数功能：处理用户的显示所有用户信息的操作
             思考处理步骤，写到函数体内注释
 * 参数：无
 *       从全局变量g_astAccounts中取信息，并显示
 * 返回值：无
 *        存款失败怎么处理？
 */
void ShowAccounts() 
{
    //(1)输出用户内容提示
    printf("\n所有账户信息:\n");
    
    //(2)检查是否有账户了？异常情况无账户的处理
    if (0 == g_iAccCount) 
    {
        printf("暂无账户信息\n");
        return;
    }
    
    //(3)打印表头
    printf("%-8s %-30s %-10s %-3s\n", "ID", "姓名", "余额","权限");
    printf("------------------------------------------------------\n");
    
    //(4)循环打印每一行
    for(int i = 0; i < g_iAccCount; i++) 
    {
        printf("%-8d %-30s %-10.2f %-10d\n",
               g_astAccounts[i].id, g_astAccounts[i].name, g_astAccounts[i].balance,g_astAccounts[i].quanxian);
    }

}
//登录账户，返回权限
int LoginAccount(int temp_id) {
    int temp_password;
    if (FindAccount(temp_id)==-1) {
        printf("没有找到账户\n");
        return 0;
    }
    else {
        while (1) {
            printf("请输入密码");
            scanf("%d", &temp_password);
            if (g_astAccounts[FindAccount(temp_id)].password==temp_password) {
                printf("欢迎用户%s \n",g_astAccounts[FindAccount(temp_id)].name);
                if (g_astAccounts[FindAccount(temp_id)].quanxian==1) {
                    printf("您目前以管理员身份登录");
                    return 1;
                }
                else {return 2;}
            }
            else {
                printf("密码错误,请重新输入\n");
            }
        }
    }
}
/* 函数功能：根据输入的用户ID找到
             思考处理步骤，写到函数体内注释
 * 参数：无
 *       用户ID跟数组ID 
 * 返回值：无
 *        存款失败怎么处理？
 */
int FindAccount(int id) 
{
    for(int i = 0; i < g_iAccCount; i++) 
    {
        if(g_astAccounts[i].id == id) return i;
    }
    
    return -1;
}
//删除账户
void DeleteAccount(int id) {
    printf("正在删除\n");
    int i=FindAccount(id);
    if (i==-1) {
        printf("有这个账户吗你就删");
    }
    else {
        printf("正在删除一下账户...");
        printf("%-8d %-30s %-10.2f %-3d\n",
                   g_astAccounts[i].id, g_astAccounts[i].name, g_astAccounts[i].balance,g_astAccounts[i].quanxian);
            for (; i < g_iAccCount-i; i++)
            {
                g_astAccounts[i]=g_astAccounts[i+1];
            }
            printf("sucess");
            SaveData();
        }
    }
void ChangePassword(int id) {
    int temp_password;
    while (1) {
        printf("请输入原密码");
        scanf("%d",&temp_password);
        if (g_astAccounts[FindAccount(id)].password==temp_password) {
            while (1) {
                printf("请输入新密码\n");
                scanf("%d",&temp_password);
                if (99999<temp_password&&temp_password<999999) {
                    printf("修改成功\n");
                    g_astAccounts[FindAccount(id)].password=temp_password;
                    SaveData();
                    break;
                }
                printf("密码不符合规范！请重新输入\n");
            }

        }
        else {
            printf("密码错误，请重新输入");
        }
    }
}