# BankManager — C 语言银行账户管理系统

## 构建 & 运行

```sh
gcc bank.c account.c transaction.c file_io.c utils.c log.c graph.c hash.c -o bank_system
./bank_system
```

`bank.h` 是唯一头文件，所有 `.c` 编译顺序无要求。

## 项目布局

所有源文件在根目录，无 `src/` 子目录。无包管理器、无构建系统、无测试框架。

| 文件 | 职责 |
|------|------|
| `bank.c` | 主循环、菜单、权限路由 |
| `account.c` | 开户、销户、登录、修改密码、查询全部账户 |
| `transaction.c` | 存款、取款、查余额、转账 |
| `file_io.c` | 二进制文件读写 (bank.dat) |
| `utils.c` | 输入校验工具（`GetInputInt`/`GetInputDouble` 已定义但未被使用） |
| `log.c` | 环形队列日志，支持 KMP 关键词搜索 |
| `graph.c` | 从转账日志建邻接矩阵，分析客户间资金来往关系 |
| `hash.c` | 哈希表，映射账户ID到数组下标，线性探测+墓碑标记 |
| `bank.h` | 唯一头文件，声明所有公有函数和类型 |

## 重要数据/约束

- **Max 50 账户** (`MAX_ACCOUNTS`)，全局数组 `g_astAccounts`，不使用 `malloc`。
- **`g_iAccCount` 初始值=1**（非0），槽位 0 预留给管理员。
- **默认管理员**: ID=1000, name="admin", password=123456, quanxian=1。首次运行时自动创建。
- **密码**: 纯 6 位数字（100000-999999）。
- **权限**: 1=管理员，2=普通用户。
- **数据持久化**: 二进制文件 `bank.dat`（已加入 `.gitignore`）。`SaveData()` 总是写入恰好 50 条记录。
- **全局日志队列**: 环形队列 `g_logQueue`，最多 100 条 (`MAX_LOGS`)，溢出时队头出队覆盖旧记录。
- **哈希表**: `HASH_SIZE=101`（质数），开放寻址法+线性探测，删除用墓碑标记（id=-2）。

## 代码约定

- 中文注释和中文菜单字符串。
- 全局变量命名前缀 `g_`（如 `g_iAccCount`, `g_astAccounts`）。
- 无测试。

## 已知陷阱

- **`InitAdminAccount()` 双重调用导致密码重置** — `LoadData()` 内无文件时调用一次，`main()` 第 21 行无条件再调用一次。管理员密码修改在重启后必然丢失。
- **`SaveData()` 内部调用 `LoadData()`**（`file_io.c:22`），写盘后立即重读以重新计数 `g_iAccCount`。注意这会导致刚保存的数组被覆盖读回。
- **`GenerateDummyData()`（graph.c）会清空全部日志并修改账户余额**，仅用于测试演示，非生产操作。
- **`DeleteAccount()` 不递减 `g_iAccCount`** — 递减由调用方 `bank.c:84` 负责。函数内 `for (; i < g_iAccCount-i; i++)` 边界有误（应为 `g_iAccCount-1`）。
- **`exit(0)` 短路** — `bank.c:53` 直接 `exit(0)`，不会执行 `main()` 末尾的清理代码（当前无实质性清理，但未来加逻辑时需注意）。
- **`ChangePassword()` 已定义但未接入菜单**（`account.c:159`），无法从界面调用。
- **`utils.c` 的 `GetInputInt`/`GetInputDouble` 已定义但未被任何调用方使用**，全部输入仍用裸 `scanf`。
