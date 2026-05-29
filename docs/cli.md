# CLI 管理工具文档

## 概述

CLI（命令行界面）管理工具是在线评测系统的管理员操作入口，与 cpp-httplib 服务平级。所有管理员操作均通过 CLI 完成，CLI 直接调用核心层接口执行管理命令。

## 架构位置

- **层级**：用户输入层（与 cpp-httplib 并列）
- **调用方式**：直接调用核心层接口
- **目标用户**：系统管理员
- **运行环境**：本地命令行环境
- **实现语言**：C++

## 主要功能

这个和oj主程序是同一个文件

### 比赛管理

- 创建比赛
- 修改比赛配置
- 删除比赛
- 查看比赛列表和详情

### 题目管理

- 添加题目到比赛
- 移除题目
- 更新题目配置
- 上传测试数据
- 修改题目描述

### 用户管理

- 创建用户账号
- 删除用户账号
- 修改用户信息
- 查看用户列表

### 缓存管理

- 手动触发缓存更新
- 使指定缓存失效
- 按模式批量清除缓存

### 判题单元管理

- 查看判题负载
- 查看队列状态
- 查看单个任务进度
- 取消队列中的任务
- 切换评测模式（FIFO/批量）

## 工作流程

### 管理员命令执行流程

```
管理员输入命令 
    → CLI 工具解析命令 
    → 调用核心层接口 
    → 核心层执行操作 
    → 返回结果给 CLI 
    → CLI 输出结果
```

### 典型操作示例

1. **创建比赛**
   ```
   oj-cli create-contest --name "比赛名称" --start-time "2024-01-01 09:00" --duration 180
   ```
2. **添加题目**
   ```
   oj-cli add-problem --contest-id 1 --name "A+B Problem" --time-limit 1000 --memory-limit 256
   ```
3. **上传测试数据**
   ```
   oj-cli upload-testdata --contest-id 1 --problem-id 1 --input 1.in --output 1.out
   ```
4. **更新缓存**
   ```
   oj-cli invalidate-cache --contest-id 1 --problem-id 1
   ```
5. **查看判题状态**
   ```
   oj-cli judger-status
   ```
6. **切换评测模式**
   ```
   oj-cli set-judge-mode --mode fifo
   oj-cli set-judge-mode --mode batch
   ```

## 技术特性

- 直接调用核心层接口，无需通过 HTTP
- 与 cpp-httplib 共享相同的业务逻辑调用入口
- 支持管理员集中管理用户账号
- 提供缓存主动失效功能
- 支持判题单元监控和管理
- 使用 C++ 实现，编译为独立可执行文件

## 与 cpp-httplib 的区别

| 特性   | CLI     | cpp-httplib      |
| ---- | ------- | ---------------- |
| 用户类型 | 管理员     | 普通用户             |
| 调用方式 | 直接调用核心层 | HTTP RESTful API |
| 主要功能 | 管理操作    | 业务操作             |
| 鉴权方式 | 管理员权限   | Session 鉴权       |
| 运行环境 | 本地命令行   | Web 服务           |

## 命令参考

### 基础命令

- `oj-cli help` - 显示帮助信息
- `oj-cli version` - 显示版本信息

### 比赛命令

- `oj-cli list-contests` - 列出所有比赛
- `oj-cli create-contest` - 创建新比赛
- `oj-cli update-contest` - 更新比赛信息
- `oj-cli delete-contest` - 删除比赛

### 题目命令

- `oj-cli list-problems` - 列出比赛题目
- `oj-cli add-problem` - 添加题目
- `oj-cli update-problem` - 更新题目
- `oj-cli delete-problem` - 删除题目
- `oj-cli upload-testdata` - 上传测试数据

### 用户命令

- `oj-cli list-users` - 列出用户
- `oj-cli create-user` - 创建用户
- `oj-cli delete-user` - 删除用户
- `oj-cli update-user` - 更新用户信息

### 缓存命令

- `oj-cli invalidate-cache` - 使缓存失效
- `oj-cli update-cache` - 更新缓存

### 判题命令

- `oj-cli judger-status` - 查看判题状态
- `oj-cli queue-status` - 查看队列状态
- `oj-cli cancel-task` - 取消评测任务
- `oj-cli set-judge-mode` - 设置评测模式

