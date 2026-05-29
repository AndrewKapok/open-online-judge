# Web 服务器文档

## 概述

Web 服务器是在线评测系统的主要 Web API 服务层，为普通用户提供 RESTful API 接口。服务器运行在用户输入层，负责收发所有 HTTP 请求，与核心层交互处理业务逻辑。

## 架构位置

- **层级**：用户输入层
- **调用方式**：HTTP RESTful API
- **目标用户**：普通用户
- **框架**：cpp-httplib (C++)

## 主要职责

### API 请求收发

- 接收所有 HTTP 请求（静态资源和 `/api/v1/*` 路径下的 API 请求）
- 解析 HTTP 请求参数和 Body
- 返回标准化的 JSON 响应或静态页面

### Cookie 解析

- 手动解析 HTTP Header 中的 Cookie
- 提取 Session ID
- 将 Session ID 传递给核心层进行验证

### 响应构造

- 将核心层处理结果构造为 HTTP 响应
- 设置适当的 HTTP 状态码
- 设置 `Set-Cookie` 响应头（如登录成功时）

## Cookie 解析方案

### 手动解析 HTTP Header

由于不使用第三方 Cookie 解析库，服务端手动解析客户端请求中的 Cookie：

1. 从 HTTP 请求头获取 `Cookie` 字段
2. 按 `;` 分割多个 Cookie
3. 查找键名为 `session_id`（或项目定义的键名）的 Cookie
4. 提取对应的 Session ID 值
5. 将 Session ID 传递给核心层验证用户身份

```cpp
std::string parse_session_id(const std::string& cookie_header) {
    std::istringstream stream(cookie_header);
    std::string cookie;
    
    while (std::getline(stream, cookie, ';')) {
        size_t eq_pos = cookie.find('=');
        if (eq_pos == std::string::npos) continue;
        
        std::string key = cookie.substr(0, eq_pos);
        std::string value = cookie.substr(eq_pos + 1);
        
        trim(key);
        trim(value);
        
        if (key == "session_id") {
            return value;
        }
    }
    return "";
}
```

## API 路由设计

### 用户认证模块

#### 注册
- **端点**：`POST /api/v1/register`
- **功能**：转发注册请求到核心层
- **输入**：用户名、密码
- **输出**：注册结果

#### 登录
- **端点**：`POST /api/v1/login`
- **功能**：转发登录请求到核心层，设置 Session Cookie
- **输入**：用户名、密码
- **输出**：登录结果，通过 `Set-Cookie` 下发 Session ID

#### 登出
- **端点**：`POST /api/v1/logout`
- **功能**：转发登出请求到核心层，清除 Session Cookie
- **输入**：无（通过 Cookie 识别用户）
- **输出**：登出结果

### 比赛模块

#### 比赛列表
- **端点**：`GET /api/v1/contests`
- **功能**：转发请求到核心层获取比赛列表
- **输出**：已开始、即将开始、已结束的比赛列表

#### 比赛详情
- **端点**：`GET /api/v1/contest/{contest_id}`
- **功能**：转发请求到核心层获取比赛详情
- **输出**：比赛名称、开始时间、赛制、题目数量等

### 题目模块

#### 题目列表
- **端点**：`GET /api/v1/contest/{contest_id}/problems`
- **功能**：转发请求到核心层获取题目列表
- **输出**：题目列表

#### 题目详情
- **端点**：`GET /api/v1/contest/{contest_id}/problem/{problem_id}`
- **功能**：转发请求到核心层获取题目详情
- **输出**：题目描述、限制条件等

### 提交模块

#### 代码提交
- **端点**：`POST /api/v1/contest/{contest_id}/submit`
- **功能**：转发提交请求到核心层
- **输入**：题目 ID、代码内容、语言
- **输出**：提交 ID、评测状态

#### 提交记录
- **端点**：`GET /api/v1/contest/{contest_id}/submissions`
- **功能**：转发请求到核心层获取提交记录
- **输出**：提交列表，包含用户、题目、状态、时间等

#### 提交详情
- **端点**：`GET /api/v1/submission/{submission_id}`
- **功能**：转发请求到核心层获取提交详情
- **输出**：各测试点状态、得分、时间、内存等

### 排行榜模块

#### 排行榜
- **端点**：`GET /api/v1/contest/{contest_id}/rank`
- **功能**：转发请求到核心层获取排行榜
- **输出**：用户排名列表，包含得分、解题数等

### 用户模块

#### 个人信息
- **端点**：`GET /api/v1/user/profile`
- **功能**：转发请求到核心层获取用户信息
- **输出**：用户名、注册时间等

#### 更新个人信息
- **端点**：`PUT /api/v1/user/profile/update`
- **功能**：转发请求到核心层更新用户信息
- **输入**：要更新的字段

## 请求处理流程

```
HTTP 请求到达
    ↓
手动解析 Cookie 提取 Session ID
    ↓
调用核心层接口（传递请求参数和 Session ID）
    ↓
核心层处理：Session 验证 → 权限检查 → 业务操作
    ↓
核心层返回处理结果
    ↓
Web 服务器构造 HTTP 响应
    ↓
返回给客户端
```

## 技术特性

- 同步阻塞 I/O（cpp-httplib 模型）
- 内置线程池支持并发请求
- RESTful API 设计
- Cookie-based Session 认证
- 手动解析 HTTP Cookie
- 跨平台支持（Windows/Linux）

## 与其他组件的交互

### 与核心逻辑层
- 调用核心层接口执行所有业务操作
- 传递 Session ID 给核心层验证
- 接收核心层处理结果并返回给客户端

### 与文件系统
- 读取静态页面文件
- 直接返回静态资源

## 错误处理

### 标准错误码

- **400 Bad Request**：请求参数错误
- **401 Unauthorized**：未登录或 Session 失效
- **403 Forbidden**：权限不足
- **404 Not Found**：资源不存在
- **500 Internal Server Error**：服务器内部错误

### 响应格式

```json
{
  "success": false,
  "error": "错误信息",
  "error_code": "ERROR_CODE"
}
```

## 性能优化

- 静态文件缓存减少磁盘 I/O
- 内置线程池提高并发处理能力
- 手动解析 Cookie 减少依赖
