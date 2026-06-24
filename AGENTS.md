# BankManager — C 语言银行账户管理系统

## 构建 & 运行

```sh
gcc bank.c account.c transaction.c file_io.c utils.c log.c graph.c hash.c -o bank_system
./bank_system
```

`bank.h` 包含所有结构体定义、函数声明、全局常量。所有 `.c` 的编译顺序无要求。

## 项目布局

所有源文件在仓库根目录，无 `src/` 子目录。无包管理器、无构建系统、无测试框架。

| 文件 | 职责 |
|------|------|
| `bank.c` | 主循环、菜单、权限路由 |
| `account.c` | 开户、销户、登录、查询全部账户 |
| `transaction.c` | 存款、取款、查余额、转账 |
| `file_io.c` | 二进制文件读写 (bank.dat) |
| `utils.c` | 输入校验工具 |
| `log.c` | 环形队列日志，支持按关键词搜索 |
| `graph.c` | 从转账日志建图，分析客户间资金来往关系 |
| `hash.c` | 哈希表，映射账户ID到数组下标，加速查找 |
| `bank.h` | 唯一头文件，声明所有公有函数和类型 |

## 重要约束

- **Max 50 账户** (`MAX_ACCOUNTS`)，全局数组 `g_astAccounts`，不使用 `malloc`。
- **默认管理员**: ID=1000, name="admin", password=123456, permission=1。系统首次运行时自动创建。
- **密码**: 纯 6 位数字。
- **权限**: 1=管理员，2=普通用户。管理员多出显示全部、删账户、创管理员、看日志功能。
- **数据持久化**: 二进制文件 `bank.dat`（已加入 `.gitignore`）。
- **全局日志队列**: 环形队列 `g_logQueue`，最多 100 条 (`MAX_LOGS`)，溢出覆盖旧记录。

## 代码约定

- 中文注释和中文菜单字符串。
- 全局变量命名前缀 `g_`（如 `g_iAccCount`, `g_astAccounts`）。
- 无测试；曾有测试文件但已被删除（commit `ce8abe3`）。

## 已知陷阱

- **`InitAdminAccount()` 每次启动都调用**，无条件将 `g_astAccounts[0]` 重置为 admin/123456。管理员密码修改在重启后丢失。
- **`GenerateDummyData()`（graph.c）会清空全部日志并修改余额**，仅用于测试演示，非生产操作。
- **`DeleteAccount()` 不递减 `g_iAccCount`**——递减操作由调用方 `bank.c:83` 负责。函数内元素前移循环 `for (; i < g_iAccCount-i; i++)` 有逻辑错误。
