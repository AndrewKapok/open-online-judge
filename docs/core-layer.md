# 核心逻辑层文档

## 概述

核心逻辑层是在线评测系统的业务规则引擎，将业务规则封装为内部接口，为 Web 服务器和 CLI 管理工具提供统一的调用入口。核心层避免了业务逻辑的重复，确保所有操作的一致性。

## 架构位置

- **层级**：核心内核层（core内核层）
- **调用者**：Web 服务器、CLI 管理工具
- **依赖**：SQLite 数据库、文件系统、判题单元

## 主要职责

### 1. 用户认证与 Session 管理

#### 用户注册
- 验证用户名合法性
- 检查用户名是否已存在
- 对密码进行加盐哈希
- 在 SQLite 数据库中创建用户记录
- 返回注册结果

#### 登录认证
- 验证用户名和密码
- 从 SQLite 数据库查询用户信息
- 密码验证通过后，在核心层内部内存中创建 Session
- 生成 Session ID 并返回

#### Session 验证与权限校验
- 验证 Session 是否存在且未过期
- 检查用户权限是否满足操作要求
- Session 续期（每次访问刷新 TTL）
- Session 销毁（登出时删除）

### 2. 比赛管理

#### 比赛创建
- 验证比赛参数（名称、时间、赛制等）
- 在文件系统中创建比赛目录
- 在 SQLite 数据库中创建比赛记录
- 初始化比赛配置

#### 比赛修改
- 更新比赛信息
- 使相关缓存失效
- 更新 SQLite 数据库记录

#### 比赛删除
- 删除比赛目录及所有子目录
- 删除 SQLite 数据库中的比赛记录
- 清理相关缓存

#### 比赛查询
- 获取比赛列表（按状态分组）
- 获取比赛详情
- 计算比赛状态（已开始/即将开始/已结束）

### 3. 题目管理

#### 题目添加
- 验证题目参数
- 在文件系统创建题目目录
  - 创建 `problem_{id}/` 目录
  - 创建 `problem.md`（题面描述）
  - 创建 `config.json`（题目配置）
  - 创建 `testdata/` 目录
- 在 SQLite 数据库中创建题目记录
- 关联题目到比赛

#### 题目修改
- 更新题目描述和配置
- 更新 `problem.md` 和 `config.json`
- 使题目页面缓存失效
- 更新 SQLite 数据库记录

#### 题目删除
- 删除题目目录
- 删除 SQLite 数据库记录
- 清理相关缓存

#### 测试数据上传
- 验证测试数据格式
- 上传到 `testdata/` 目录
- 更新 `config.json` 中的测试点数量
- 使相关缓存失效

### 4. 提交评测

#### 提交入队
- 验证提交参数（题目 ID、代码、语言）
- 在文件系统保存代码
  - 路径：`contest_{id}/submissions/user_{user_id}/problem_{id}/submission_{n}.ext`
- 在 SQLite 数据库创建提交记录
- 将评测任务投递到消息队列
- 返回提交 ID

#### 结果查询
- 从 SQLite 数据库查询提交记录
- 获取各测试点状态
- 计算总分和详细信息

### 5. 排名计算

- 根据比赛赛制计算排名
- 统计用户解题数和得分
- 按规则排序（如 AC 数、总时间等）
- 返回排名列表

### 6. 缓存管理

#### 缓存主动失效
- 题目更新时使页面缓存失效
- 配置更新时使相关缓存失效
- 支持按模式批量清除缓存
- Session 存储和管理
- 消息队列操作
- 判题单元状态跟踪

### 7. 判题配置

#### 配置读取
- 从文件系统 `config.json` 加载判题配置
- 包括：测试点数量、时空限制、编译参数、比对方式等

#### 配置更新
- 更新题目配置
- 验证配置合法性
- 使相关缓存失效

## 调用接口设计

### 接口设计原则

所有需要认证的接口都接收 `session_id` 参数，核心层内部进行 Session 验证和权限校验。

### 用户相关接口

```cpp
// 用户注册（无需认证）
struct RegisterResult {
    bool success;
    std::string error;
    int user_id;
};
RegisterResult register_user(const std::string& username, const std::string& password);

// 用户登录（无需认证）
struct LoginResult {
    bool success;
    std::string error;
    std::string session_id;
};
LoginResult login_user(const std::string& username, const std::string& password);

// 用户登出（需要认证）
bool logout_user(const std::string& session_id);

// 验证 Session（内部使用）
struct UserInfo {
    int user_id;
    std::string username;
    std::string role;
};
std::optional<UserInfo> validate_session(const std::string& session_id);

// 获取用户信息（需要认证）
std::optional<UserInfo> get_user_profile(const std::string& session_id);

// 更新用户信息（需要认证）
struct UpdateProfileResult {
    bool success;
    std::string error;
};
UpdateProfileResult update_user_profile(const std::string& session_id, const nlohmann::json& updates);
```

### 比赛相关接口

```cpp
// 创建比赛（需要管理员权限）
struct ContestConfig {
    std::string name;
    std::string start_time;
    int duration;
    std::string format;
};
std::optional<int> create_contest(const std::string& session_id, const ContestConfig& config);

// 更新比赛（需要管理员权限）
struct ContestUpdate {
    std::optional<std::string> name;
    std::optional<std::string> start_time;
    std::optional<int> duration;
};
bool update_contest(const std::string& session_id, int contest_id, const ContestUpdate& updates);

// 删除比赛（需要管理员权限）
bool delete_contest(const std::string& session_id, int contest_id);

// 获取比赛列表（需要登录）
struct ContestInfo {
    int id;
    std::string name;
    std::string start_time;
    std::string status;
};
std::vector<ContestInfo> list_contests(const std::string& session_id);

// 获取比赛详情（需要登录）
struct ContestDetail {
    int id;
    std::string name;
    std::string description;
    std::string start_time;
    std::string end_time;
    std::string format;
    std::vector<int> problem_ids;
};
std::optional<ContestDetail> get_contest(const std::string& session_id, int contest_id);
```

### 题目相关接口

```cpp
// 添加题目（需要管理员权限）
struct ProblemConfig {
    std::string name;
    int time_limit;
    int memory_limit;
    int test_cases;
};
std::optional<int> add_problem(const std::string& session_id, int contest_id, const ProblemConfig& config);

// 更新题目（需要管理员权限）
struct ProblemUpdate {
    std::optional<std::string> description;
    std::optional<ProblemConfig> config;
};
bool update_problem(const std::string& session_id, int contest_id, int problem_id, const ProblemUpdate& updates);

// 删除题目（需要管理员权限）
bool delete_problem(const std::string& session_id, int contest_id, int problem_id);

// 获取题目列表（需要登录）
struct ProblemInfo {
    int problem_id;
    std::string title;
    int order;
};
std::vector<ProblemInfo> list_problems(const std::string& session_id, int contest_id);

// 获取题目详情（需要登录）
struct ProblemDetail {
    int problem_id;
    std::string description;
    ProblemConfig config;
};
std::optional<ProblemDetail> get_problem(const std::string& session_id, int contest_id, int problem_id);

// 上传测试数据（需要管理员权限）
struct TestData {
    int test_case_id;
    std::string input;
    std::string output;
};
bool upload_testdata(const std::string& session_id, int contest_id, int problem_id, const std::vector<TestData>& testdata);
```

### 提交相关接口

```cpp
// 提交代码（需要登录）
struct SubmissionResult {
    bool success;
    std::string error;
    int submission_id;
};
SubmissionResult submit_code(const std::string& session_id, int contest_id, int problem_id, 
                             const std::string& code, const std::string& language);

// 获取提交记录（需要登录）
struct SubmissionInfo {
    int id;
    int user_id;
    int problem_id;
    std::string language;
    std::string status;
    int score;
    std::string created_at;
};
std::vector<SubmissionInfo> get_submissions(const std::string& session_id, int contest_id, std::optional<int> user_id);

// 获取提交详情（需要登录）
struct TestCaseResult {
    int testcase;
    std::string result;
    std::string time;
    std::string memory;
};
struct SubmissionDetail {
    int id;
    int user_id;
    int problem_id;
    int contest_id;
    std::string language;
    std::string status;
    int score;
    std::string time_usage;
    std::string memory_usage;
    std::vector<TestCaseResult> details;
    std::string created_at;
};
std::optional<SubmissionDetail> get_submission_detail(const std::string& session_id, int submission_id);
```

### 排名相关接口

```cpp
// 获取排行榜（需要登录）
struct RankingInfo {
    int rank;
    int user_id;
    std::string username;
    int score;
    int solved;
};
std::vector<RankingInfo> get_ranking(const std::string& session_id, int contest_id);
```

### 缓存相关接口

```cpp
// 使缓存失效
bool invalidate_cache(const std::string& key);

// 批量使缓存失效
int invalidate_cache_pattern(const std::string& pattern);

// 更新缓存
bool update_cache(const std::string& key, const std::string& value, int ttl);
```

## 与其他组件的交互

### 与 SQLite 数据库
- 用户账户数据读写（SQLite3 C API）
- 比赛记录管理
- 提交记录存储
- 排行榜数据

### 与文件系统
- 题目目录管理
- 测试数据存储
- 用户代码保存
- 配置文件读写

### 与判题单元
- 投递评测任务
- 查询判题状态
- 更新评测结果

## 进程间通信

系统内部模块通过 Unix Domain Socket 配合守护进程进行通信：

- **通信方式**：同步或异步请求
- **数据格式**：JSON（rapidjson 序列化/反序列化）
- **特点**：高性能，支持并发连接，跨平台

## 设计原则

1. **统一接口**：Web 服务器和 CLI 共享相同的调用入口
2. **职责明确**：Web 服务器只负责收发请求，核心层负责业务逻辑和权限校验
3. **Session 中心化**：所有 Session 验证和权限检查在核心层内部完成
4. **避免重复**：业务逻辑只在核心层实现一次
5. **一致性**：确保所有操作的结果一致
6. **可维护性**：清晰的接口设计，易于扩展和维护
7. **高性能**：优化数据访问和缓存策略
