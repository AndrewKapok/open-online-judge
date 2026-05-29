# 数据存储文档

## 概述

在线评测系统采用混合存储方案：SQLite 数据库存储结构化数据（用户账户、提交记录等），文件系统存储非结构化数据（题目描述、测试数据、用户代码等）。这种设计兼顾了查询性能和数据管理的便利性。

## SQLite 数据库

### 存储引擎

- **数据库类型**：SQLite（轻量级嵌入式数据库）
- **操作方式**：SQLite3 C API
- **部署方式**：单文件数据库
- **适用场景**：中小规模用户和提交记录

### 用户账户表

#### 表结构：users

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY, AUTOINCREMENT | 用户 ID（主键） |
| username | TEXT | UNIQUE, NOT NULL | 用户名（唯一） |
| password_hash | TEXT | NOT NULL | 加盐哈希密码 |
| salt | TEXT | NOT NULL | 密码盐值 |
| created_at | TEXT | NOT NULL | 注册时间 |
| role | TEXT | NOT NULL, DEFAULT 'user' | 用户角色（user/admin） |

#### 密码存储

- **加密方式**：加盐哈希存储
- **不保存明文**：确保密码安全
- **盐值**：每个用户独立的随机盐值
- **哈希算法**：推荐使用安全的哈希算法（如 bcrypt、PBKDF2）

#### 用户注册流程

1. 验证用户名唯一性
2. 生成随机盐值
3. 对密码进行加盐哈希
4. 插入数据库记录
5. 返回用户 ID

#### 登录验证流程

1. 根据用户名查询用户记录
2. 使用存储的盐值对输入密码进行哈希
3. 比对哈希值与存储的 password_hash
4. 验证通过则创建 Session

### Session 管理

- **存储位置**：核心层内部内存（不落数据库）
- **服务重启后失效**：无需持久化
- **有效期内自动续期**：每次访问刷新 TTL

### 提交记录表

#### 表结构：submissions

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY, AUTOINCREMENT | 提交 ID（主键） |
| user_id | INTEGER | NOT NULL, FOREIGN KEY | 用户 ID |
| contest_id | INTEGER | NOT NULL | 竞赛 ID |
| problem_id | INTEGER | NOT NULL | 题目 ID |
| submission_time | TEXT | NOT NULL | 提交时间 |
| code_path | TEXT | NOT NULL | 代码文件路径 |
| language | TEXT | NOT NULL | 编程语言 |
| status | TEXT | NOT NULL | 整体状态（AC/WA/TLE等） |
| total_score | INTEGER | NOT NULL, DEFAULT 0 | 总得分 |
| total_time | INTEGER | NOT NULL, DEFAULT 0 | 总耗时（毫秒） |
| total_memory | INTEGER | NOT NULL, DEFAULT 0 | 总内存使用 |

#### 提交记录字段说明

- **提交 ID**：主键，唯一标识每次提交
- **用户 ID**：关联 users 表
- **题目 ID**：由竞赛 ID + 题目 ID 唯一确定
- **提交时间**：ISO 格式的时间字符串
- **代码文件路径**：指向文件系统中实际存储位置
- **评测结果详情**：每个测试点的通过/失败状态、得分、时间、内存等

### 测试点结果表

#### 表结构：test_case_results

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY, AUTOINCREMENT | 记录 ID（主键） |
| submission_id | INTEGER | NOT NULL, FOREIGN KEY | 提交 ID |
| test_case_id | INTEGER | NOT NULL | 测试点 ID |
| status | TEXT | NOT NULL | 状态（AC/WA/TLE/MLE/RE） |
| score | INTEGER | NOT NULL | 该测试点得分 |
| time | INTEGER | NOT NULL | 执行时间（毫秒） |
| memory | INTEGER | NOT NULL | 内存使用（KB） |
| message | TEXT | | 附加信息（错误信息等） |

### 比赛表

#### 表结构：contests

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | INTEGER | PRIMARY KEY, AUTOINCREMENT | 比赛 ID（主键） |
| name | TEXT | NOT NULL | 比赛名称 |
| start_time | TEXT | NOT NULL | 开始时间 |
| duration | INTEGER | NOT NULL | 比赛时长（分钟） |
| format | TEXT | NOT NULL | 赛制（ICPC/OI等） |
| created_at | TEXT | NOT NULL | 创建时间 |
| created_by | INTEGER | NOT NULL, FOREIGN KEY | 创建者 ID |

### 排行榜计算

- **数据来源**：submissions 表和 test_case_results 表
- **计算方式**：根据比赛赛制计算
- **ICPC 赛制**：按 AC 数量排序，AC 数量相同按总时间排序
- **OI 赛制**：按总得分排序

## 文件系统

### 目录结构

采用清晰的目录结构，便于管理与备份：

```
数据根目录/
├─ contest_{id1}/
│  ├─ problems/
│  │  ├─ problem_{id1}/
│  │  │  ├─ problem.md           # 题面描述
│  │  │  ├─ config.json          # 题目配置
│  │  │  └─ testdata/            # 测试数据
│  │  │     ├─ 1.in / 1.out
│  │  │     ├─ 2.in / 2.out
│  │  │     └─ ...
│  │  └─ problem_{id2}/
│  │     └─ ...
│  └─ submissions/
│     ├─ user_{user_id}/
│     │  ├─ problem_{id1}/
│     │  │  ├─ submission_1.cpp
│     │  │  ├─ submission_2.cpp
│     │  │  └─ ...
│     │  └─ problem_{id2}/
│     └─ user_{user_id2}/
├─ contest_{id2}/
└─ ...
```

### 题目存储

#### 题目目录结构

每个题目包含以下文件：

- **problem.md**：Markdown 格式的题面描述
- **config.json**：题目配置文件（JSON 格式）
- **testdata/**：测试数据目录

#### problem.md

- **格式**：Markdown
- **内容**：题目描述、输入输出格式、样例、数据范围等
- **渲染**：前端或后端渲染为 HTML

#### config.json

```json
{
  "problem_id": 1,
  "contest_id": 1,
  "time_limit": 1000,
  "memory_limit": 256,
  "test_cases": 10,
  "languages": ["C", "C++", "Java", "Python3"],
  "compiler_flags": {
    "C": "-O2 -std=c99",
    "C++": "-O2 -std=c++17"
  },
  "comparator": "default",
  "input_output": {
    "type": "stdio",
    "input_file": null,
    "output_file": null
  }
}
```

#### 测试数据

- **命名规则**：`{test_case_id}.in` 和 `{test_case_id}.out`
- **格式**：纯文本文件
- **输入文件**：程序的标准输入
- **输出文件**：期望的程序输出

### 用户代码存储

#### 代码目录结构

```
contest_{id}/submissions/user_{user_id}/problem_{problem_id}/
```

#### 代码文件命名

- **格式**：`submission_{submission_id}.{extension}`
- **扩展名**：根据编程语言确定
  - C: `.c`
  - C++: `.cpp`
  - Java: `.java`
  - Python: `.py`

#### 代码保存流程

1. 用户提交代码
2. 核心层生成提交 ID
3. 根据目录结构创建路径
4. 将代码写入文件
5. 在 SQLite 数据库记录文件路径

### 数据备份

#### 备份策略

- **文件系统**：直接拷贝数据目录
- **SQLite 数据库**：使用 SQLite 备份 API 或导出 SQL 文件

#### 备份内容

- 所有比赛目录
- 题目描述和配置
- 测试数据
- 用户代码
- 提交记录

## 数据一致性

### 事务保证

- **SQLite 事务**：使用事务保证多表操作的原子性
- **文件系统操作**：先写文件，再更新数据库

### 一致性维护

- **缓存失效**：数据更新时使相关缓存失效
- **级联删除**：删除比赛时删除所有相关题目和提交
- **外键约束**：使用外键保证引用完整性

## 性能优化

### SQLite 优化

- **索引**：为常用查询字段创建索引
- **连接复用**：复用数据库连接
- **批量操作**：批量插入和更新

### 文件系统优化

- **缓存**：题目描述和配置缓存
- **延迟写入**：非关键数据延迟写入
- **文件压缩**：历史数据压缩存储

## 安全考虑

### 密码安全

- **加盐哈希**：不存储明文密码
- **安全哈希算法**：使用 bcrypt 或 PBKDF2
- **盐值唯一**：每个用户独立的随机盐值

### 代码安全

- **权限控制**：限制文件系统访问
- **代码隔离**：每个用户的代码独立存储

### 数据保护

- **备份策略**：定期备份数据
- **访问控制**：只有核心层可修改数据
- **审计日志**：记录重要操作

## 扩展性

### 数据库扩展

- **迁移支持**：可迁移到 PostgreSQL 或 MySQL
- **分表策略**：提交记录可按比赛分表
- **读写分离**：读操作可使用只读副本

### 文件系统扩展

- **对象存储**：可迁移到 S3 或 OSS
- **CDN 加速**：静态资源使用 CDN
- **分布式存储**：大规模场景使用分布式文件系统
