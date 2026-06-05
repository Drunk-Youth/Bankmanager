# BankManager

Windows 命令行银行账户管理系统，使用 C 语言实现，支持账户管理、交易记录和基于 Merkle 树的数据完整性校验。

## 构建

```powershell
gcc *.c -o bank.exe -ladvapi32
```

运行：`.\bank.exe`

---

## 数据结构总览

项目围绕 **4 个核心数据结构** 构建，它们共同决定了系统的存储、操作和校验能力：

| 数据结构 | 定义位置 | 用途 |
|----------|----------|------|
| `STACCOUNT` | `bank.h:8-15` | 账户信息记录 |
| `LogEntry` | `bank.h:55-62` | 单笔交易日志 |
| `LogQueue` | `bank.h:64-69` | 交易日志的环形缓冲队列 |
| `HashValue` | `bank.h:87-89` | SHA-256 哈希值（Merkle 树节点） |

---

## 1. STACCOUNT — 账户信息

```c
#define MAX_ACCOUNTS 50
#define NAME_LEN      31

typedef struct strAccount {
    int    id;                    // 唯一账户 ID
    char   name[NAME_LEN];       // 户名（最长 30 字符）
    double balance;              // 账户余额
    int    password;             // 6 位数字密码 (100000–999999)
    int    quanxian;             // 权限级别: 0=访客, 1=管理员, 2=普通用户
} STACCOUNT;
```

### 全局实例

```c
STACCOUNT g_astAccounts[MAX_ACCOUNTS];  // bank.c:6 — 固定 50 槽位数组
int       g_iAccCount;                  // bank.c:8 — 当前有效账户数量
```

### 设计要点

- **固定容量**：最多 50 个账户，由 `MAX_ACCOUNTS` 宏控制。
- **删除策略**：删除账户时将其槽位**全部置零**（`memset`），而非移动元素。`LoadData` 通过统计 `id != 0` 的槽位重新计算 `g_iAccCount`。
- **管理员硬编码**：id=1000, password=123456，`InitAdminAccount()` 每次启动都会强制覆盖第一个槽位（`account.c:18-22`）。
- **内存布局**：`sizeof(STACCOUNT)` = 4 + 31 + 3(padding) + 8 + 4 + 4 = 54 字节（假设 64 位, 对齐后 56）。

---

## 2. LogEntry — 交易日志条目

```c
typedef struct {
    int    account_id;      // 关联的账户 ID
    char   type[10];        // 操作类型: "Deposit" 或 "Withdraw"
    double amount;          // 交易金额
    double balance;         // 交易后余额
    char   timestamp[20];   // 时间戳 (格式: YYYY-MM-DD HH:MM:SS)
    char   location[50];    // 地点信息 (固定 "宇宙总行")
} LogEntry;
```

### 数据来源

- `type`、`amount`、`balance` 由 `Deposit()` / `Withdraw()` 填充。
- `timestamp` 由 `EnqueueLog()` 调用时通过 `time()` + `localtime()` 生成。
- `location` 在 `log.c:55` 硬编码为 `"宇宙总行"`。
- **不可变**：一旦入队，LogEntry 不再被修改（除非被环形队列覆盖）。

---

## 3. LogQueue — 环形缓冲队列

```c
#define MAX_LOGS 100

typedef struct {
    LogEntry logs[MAX_LOGS];  // 固定 100 条日志存储
    int      front;           // 队头指针（最旧记录）
    int      rear;            // 队尾指针（下一插入位）
} LogQueue;
```

### 全局实例

```c
LogQueue g_logQueue;  // bank.c:5
```

### 工作原理

```
初始状态:  front = 0, rear = 0
入队 3 条: front = 0, rear = 3,  有效记录: [0, 1, 2]
入队 100 条后继续入队: front = 1, rear = 0, 有效记录: [1..99, 0]（循环覆盖）
```

- **满判断**：`(rear + 1) % MAX_LOGS == front` — 始终保留一个空位以区分满/空。
- **空判断**：`front == rear`。
- **容量上限**：`MAX_LOGS - 1 = 99` 条有效记录。
- **溢出行为**：当队列满时，`EnqueueLog()` 静默覆盖最旧记录，并同步更新 Merkle 根。
- **遍历逻辑**：`ShowTransactionLogs()` 从 `front` 到 `(rear - 1) % MAX_LOGS` 依次输出。

### 持久化

整个 `LogQueue` 结构体以二进制形式写入 `log.dat`：

```
文件大小 = sizeof(LogQueue) = 100 * sizeof(LogEntry) + 2 * sizeof(int)
         ≈ 100 * 96 + 8 = 9608 字节
```

---

## 4. HashValue — SHA-256 哈希值

```c
#define HASH_SIZE 32

typedef struct {
    unsigned char data[HASH_SIZE];  // 32 字节 RAW SHA-256 摘要
} HashValue;
```

### 全局实例

```c
HashValue g_merkleRoot;  // merkle.c:8 — 所有日志的 Merkle 树根哈希
```

### 依赖

- 使用 **Windows CryptoAPI** (`wincrypt.h`) 计算 SHA-256。
- 编译时需链接 `-ladvapi32`。

### Merkle 树构建流程 (`BuildMerkleTree`)

1. 读取 `g_logQueue` 中的所有有效日志条目（从 `front` 到 `rear`）。
2. 对每条 `LogEntry` 调用 `HashLogEntry()`，生成 **leaf hashes**（叶子节点）。
3. 将所有叶子哈希存储在**动态分配的 `HashValue* leaves` 数组**中（堆内存，使用后释放）。
4. 自底向上两两哈希合并，直到产生唯一的根哈希 → `g_merkleRoot`。
5. 持久化到 `merkle.dat`（32 字节固定大小）。

```
日志条目: [E1]  [E2]  [E3]  [E4]  [E5]
          │     │     │     │     │
叶子哈希: [H1]  [H2]  [H3]  [H4]  [H5]
          │     │     │     │     │
中间层:   [H12]       [H34]       H5
          │           │           │
顶层:     [    H1234   ]          H5
          │                      │
Merkle根: [       H12345          ]  → g_merkleRoot
```

---

## 数据流与文件布局

```
┌──────────┐  二进制块 (50 × sizeof(STACCOUNT))    ┌──────────┐
│ bank.dat │ ◄──────────────────────────────────► │ accounts │
│  2700 B  │                                      │ 内存数组  │
└──────────┘                                      └────┬─────┘
                                                        │
┌──────────┐  sizeof(LogQueue) 二进制块    ┌────────────┐ │
│ log.dat  │ ◄──────────────────────────► │ logQueue   │◄┘
│ ≈9608 B  │                              │ 环形队列   │  交易操作
└──────────┘                              └──┬─────────┘  触发日志
                                             │
┌──────────┐  32 字节固定块                 │ BuildMerkleTree()
│merkle.dat│ ◄───────────────── g_merkleRoot│
│   32 B   │                               │
└──────────┘                               │
       ▲                                   │
       └────────── VerifyMerkleTree() ──────┘
```

### 文件一览

| 文件 | 大小 | 格式 | 内容 |
|------|------|------|------|
| `bank.dat` | 2700+ B | 二进制 | 50 个 `STACCOUNT` 连续存储 |
| `log.dat` | ~9608 B | 二进制 | 完整 `LogQueue` 结构体 |
| `merkle.dat` | 32 B | 二进制 | Merkle 根哈希 |

---

## 辅助数据结构

### KMP 部分匹配表（`log.c:111`）

```c
int* lps = (int*)malloc(M * sizeof(int));  // 动态分配，搜索后释放
```

用于在日志中执行模式搜索（`SearchLogs` → `KMPSearch`），支持任意字符串模式匹配，而非简单的 `strstr`。

### 全局状态变量

| 变量 | 类型 | 位置 | 说明 |
|------|------|------|------|
| `quanxian` | `int` | `bank.c:9` | 当前会话权限：0=未登录, 1=管理员, 2=用户, -2=退出 |
| `user_id` | `int` | `bank.c:10` | 当前登录用户 ID |
| `g_iAccCount` | `int` | `bank.c:8` | 有效账户数（不含已删除的空槽） |

---

## 已知限制

- **Windows 独占**：依赖 `<io.h>` 和 Windows CryptoAPI，无法在 POSIX 系统编译。
- **固定容量**：账户 50 个，日志 99 条有效记录，超出后日志自动覆盖。
- **明文密码**：密码以 `int` 类型存储，无哈希保护。
- **管理员硬编码**：每次启动强制重置，不支持持久化管理员的密码修改。
- **单用户会话**：`quanxian` 和 `user_id` 是全局变量，不支持多用户并发。
