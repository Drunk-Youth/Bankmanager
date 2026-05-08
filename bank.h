#ifndef BANK_H
#define BANK_H
#define MAX_LOGS 100 // 日志队列最大容量，防止无限增长
#define MAX_ACCOUNTS 50   //系统规格为50个账户
#define NAME_LEN 31       //用户名字最长30字符，因此需要多1个字符串结束符

// 账户信息，每个账户id对应了名字、账户余额等信息，是不是用数据结构定义比较合适？
typedef struct strAccount
{
    int id;
    char name[NAME_LEN];
    double balance;
    int password;
    int quanxian;//1,admin:2,user
} STACCOUNT;

// strAccount STACCOUNT;
    
// 函数声明，main函数中调用的各模块函数需要在程序公用的.h中声明
// 公开出来被其他模块调用的函数，也应该在公用的.h中声明

//创建账户的函数
void CreateAccount(int quanxian);

//实现存款的函数
void Deposit(int id);

//实现取款的函数
void Withdraw(int id);

//查询余额的函数
void QueryBalance(int id);

//显示全部账户的函数
void ShowAccounts();

//数据持久化函数
void SaveData();

//加载数据的函数
void LoadData();

//根据ID查找账户数组id的函数
int FindAccount(int iD);

//登录函数，返回用户权限
int LoginAccount(int temp_id);

//销户
void DeleteAccount(int id);

void InitAdminAccount();

// 日志条目结构
typedef struct {
    int account_id;      // 涉及的账户ID
    char type[10];       // 操作类型: "Deposit" 或 "Withdraw"
    double amount;       // 交易金额
    double balance;      // 交易后的余额
    char timestamp[20];  // 时间戳 (格式: YYYY-MM-DD HH:MM:SS)
    char location[50];   // 地点信息
} LogEntry;

// 环形队列结构
typedef struct {
    LogEntry logs[MAX_LOGS];
    int front; // 队头指针
    int rear;  // 队尾指针
} LogQueue;

// 全局日志队列变量 (在 bank.c 中定义，在其他文件中 extern 引用)
extern LogQueue g_logQueue;

// 日志函数声明
void InitLogQueue();                    // 初始化队列
int IsLogQueueFull();                   // 检查队列是否满
int IsLogQueueEmpty();                  // 检查队列是否空
int EnqueueLog(int id, const char* type, double amt, double bal); // 入队
void ShowTransactionLogs();             // 显示所有日志
#endif
