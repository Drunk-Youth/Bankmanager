#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bank.h"
LogQueue g_logQueue; // <-- 新增：定义全局日志队列
STACCOUNT g_astAccounts[MAX_ACCOUNTS];

int g_iAccCount = 1;
int quanxian =0;//1,admin:2,user
int user_id;
int d_id;
int main(void)
{
        LoadData();
        //(1)导入数据
        // which fuction? 
        
        //应该先导入数据再欢迎？还是欢迎了再导入数据？
        printf("=== 简易银行账户管理系统 ===\n");
        printf("欢迎使用! 系统已加载%d个账户\n", g_iAccCount);
        InitAdminAccount();//初始化管理员账号
        InitLogQueue();
        int iChoice=-1; //选择标志
        int pChoice=-1;
        while (1) {
            //持续显示菜单的秘诀
            //(2)显示主菜单，并且处理用户的选择
            if (quanxian==-2){break;}
            printf("\n=== 简易银行账户管理系统 ===\n");
            switch(quanxian) {
                case 0:
                printf("1. 开户\n2. 登录 \n0. 退出\n");
                //printf("1. 开户\n2. 存款\n3. 取款\n4. 查询\n5. 显示所有账户\n6. 退出\n");
                printf("请选择: ");
                iChoice=-1;
                scanf("%d", &iChoice);
                switch(iChoice) {
                    case 1://CREATE
                        //调用创建账号的函数
                        CreateAccount(2);
                        quanxian=0;
                        break;
                    case 2://LOGIN
                        printf("请输入账户id");
                        scanf("%d", &user_id);
                        quanxian=LoginAccount(user_id);
                        break;
                    case 0://EXIT
                        printf("谢谢使用!\n按任意键退出\n");
                        getchar();
                        quanxian=-2;
                        exit(0);

                        //还要实现哪些菜单吗？
                        //如果要做排序展示，要做什么？
                    default: //好的编码习惯，对无效值做处理
                        printf("无效选择!\n");
                        break; //好的编码加上break，万一default挪了个位置呢？
                }
                    break;
                case 1:
                    printf("1. 存款\n2. 取款\n3. 查询\n4. 显示所有账户\n5. 删除账户\n6. 创建管理员账户\n7.查看交易日志\n8.搜索交易日志\n9.转账\n10.资金关系图分析\n0. 退出账户");
                    pChoice=-1;
                    scanf("%d", &pChoice);

                    switch(pChoice) {
                        case 1:
                            Deposit(user_id);
                            break;
                        case 2:
                            Withdraw(user_id);
                            break;
                        case 3:
                            QueryBalance(user_id);
                            break;
                        case 4:
                            ShowAccounts();
                            break;
                        case 5:
                            printf("请输入要删除的id");
                            scanf("%d", &d_id);
                            DeleteAccount(d_id);
							g_iAccCount--;
                            break;
                        case 6:
                            CreateAccount(1);
                            break;
                        case 7:
                            ShowTransactionLogs();
                            break;
                        case 8:
                            printf("请输入搜索内容");
                            {
                            char pattern[50];
                            scanf("%s", pattern);
                            SearchLogs(pattern);
                            }
                            break;
                        case 9:
                            Transfer(user_id);
                            break;
                        case 10:
                            {
                            int g_choice;
                            do {
                                printf("\n=== 资金关系图分析 ===\n");
                                printf("1. 显示全部转账关系图\n");
                                printf("2. 查看指定账户关联关系\n");
                                printf("3. 查看最活跃账户排名\n");
                                printf("4. 查找转账路径\n");
                                printf("5. 生成虚拟交易记录\n");
                                printf("0. 返回\n");
                                printf("请选择: ");
                                scanf("%d", &g_choice);
                                switch(g_choice) {
                                    case 1:
                                        ShowTransferGraph();
                                        break;
                                    case 2: {
                                        int search_id;
                                        printf("请输入账户ID: ");
                                        scanf("%d", &search_id);
                                        ShowAccountRelations(search_id);
                                        break;
                                    }
                                    case 3:
                                        ShowMostActiveNodes();
                                        break;
                                    case 4: {
                                        int from, to;
                                        printf("请输入起始账户ID: ");
                                        scanf("%d", &from);
                                        printf("请输入目标账户ID: ");
                                        scanf("%d", &to);
                                        FindTransferPaths(from, to);
                                        break;
                                    }
                                    case 5:
                                        GenerateDummyData();
                                        break;
                                    case 0:
                                        break;
                                    default:
                                        printf("无效选择!\n");
                                        break;
                                }
                            } while (g_choice != 0);
                            }
                            break;
                        case 0://EXIT
                            printf("谢谢使用!\n按任意键退出");
                            getchar();
                            quanxian=-2;
                            //还要实现哪些菜单吗？
                            //如果要做排序展示，要做什么？
                        default: //好的编码习惯，对无效值做处理
                            printf("无效选择!\n");
                            break; //好的编码加上break，万一default挪了个位置呢？

                    }
                    break;
                case 2:
                    printf("1. 存款\n2. 取款\n3. 查询\n4.转账\n0.退出账户");
                    pChoice=-1;
                    scanf("%d", &pChoice);
                    switch(pChoice) {
                        case 1:
                            Deposit(user_id);
                            break;
                        case 2:
                            Withdraw(user_id);
                            break;
                        case 3:
                            QueryBalance(user_id);
                            break;
                        case 4:
                            Transfer(user_id);
                            break;
                        case 0://EXIT
                            printf("谢谢使用!\n按任意键退出");
                            getchar();
                            quanxian=-2;
                            break;
                            //还要实现哪些菜单吗？
                            //如果要做排序展示，要做什么？
                        default: //好的编码习惯，对无效值做处理
                            printf("无效选择!\n");
                            break; //好的编码加上break，万一default挪了个位置呢？
                    }
                    break;
            }
        }

        
        //(3)调用保存数据的函数
        // which fuction? 
        
        return 0;
}
