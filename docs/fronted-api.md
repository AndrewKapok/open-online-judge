# 在线评测系统 API 文档

所有 API 端点由 cpp-httplib 服务提供，分为两种权限级别：普通用户（登录后）和管理员

## 基础约定

- 所有路径以 `/api/v1` 为前缀（便于版本管理）
- 认证方式：基于 Cookie 的 Session 认证
  - 登录成功后，服务端在响应中 `Set-Cookie`，后续请求自动携带
  - 服务端手动解析 HTTP Cookie 提取 Session ID 进行鉴权
- 权限层级：普通用户能用的管理员都能使用，管理员能使用的不一定普通用户可以使用
- 服务端框架：cpp-httplib（C++）

---

登录部分

### POST `/api/v1/register`
- **说明**：注册新用户
- **权限**：仅未登录状态可用
- **请求体**：
```json
{
  "user": "string",
  "password": "string"
}
```
- **返回体**：
```json
{
  "info": "success"
}
```
> 若非 `success`，则返回错误信息

### POST `/api/v1/login`
- **说明**：登录，成功后设置 Session Cookie
- **权限**：仅未登录状态可用
- **请求体**：
```json
{
  "user": "string",
  "password": "string"
}
```
- **返回体**：
```json
{
  "info": "success"
}
```
> 同时在响应头的 `Set-Cookie` 中下发 Session ID。
> 若非 `success`，则返回错误信息

### POST `/api/v1/logout`
- **说明**：登出，销毁当前 Session
- **权限**：仅登录状态可用
- **请求体**：无
- **返回体**：
```json
{
  "info": "success"
}
```

用户配置部分

### GET `/api/v1/user/profile`
- **说明**：获取当前登录用户的基本信息
- **权限**：登录
- **请求体**：无
- **返回体**：
```json
{
  "id": 1,
  "username": "alice",
  "role": "user",
  "created_at": "2026-01-01T12:00:00Z"
}
```

### POST `/api/v1/user/profile/update`
- **说明**：修改个人信息（昵称、密码等）
- **权限**：登录
- **请求体**（至少提供一个字段）：
```json
{
  "password": "newpassword"
}
```
- **返回体**：
```json
{
  "info": "profile updated"
}
```
> 若非 `profile updated`，则返回错误信息

---

### GET `/api/v1/languages`
- **说明**：获取当前系统支持的编程语言列表
- **权限**：登录
- **请求体**：无
- **返回体**：
```json
{
  "languages": [
    { "id": 1, "name": "C++", "version": "gcc 11.2" },
    { "id": 2, "name": "Python", "version": "3.10" },
    { "id": 3, "name": "Java", "version": "openjdk 17" }
  ]
}
```

### GET `/api/v1/contests`
- **说明**：获取比赛列表（公开赛、进行中、已结束等）
- **权限**：登录
- **请求体**：无
- **返回体**：
```json
{
  "contests": [
    {
      "id": 1,
      "name": "春季赛",
      "start_time": "2026-04-01T10:00:00Z",
      "end_time": "2026-04-01T14:00:00Z",
      "status": "running"
    },
    {
      "id": 2,
      "name": "训练赛#3",
      "start_time": "2026-05-01T08:00:00Z",
      "end_time": "2026-05-01T12:00:00Z",
      "status": "upcoming"
    },
    {
      "id": 3,
      "name": "比赛3",
      "start_time": "2026-01-01T08:00:00Z",
      "end_time": "2026-01-02T08:00:00Z",
      "status": "ending"
    }
  ]
}
```

### GET `/api/v1/contest/{contest_id}`
- **说明**：获取比赛详细信息
- **权限**：登录
- **路径参数**：`contest_id` – 比赛 ID
- **请求体**：无
- **返回体**：
```json
{
  "id": 1,
  "name": "春季赛",
  "description": "2026年春季积分赛",
  "start_time": "2026-04-01T10:00:00Z",
  "end_time": "2026-04-01T14:00:00Z",
  "is_public": true,
  "problems": [101, 102, 103]
}
```

---

### GET `/api/v1/contest/{contest_id}/submissions`
- **说明**：查询提交列表（可按题目、状态等过滤）
- **权限**：登录，管理员看到全部，普通用户只看到自己的
- **请求体**：无（可使用查询参数，例如 `?problem_id=100&status=AC`）
- **返回体**：
```json
{
  "submissions": [
    {
      "id": 2001,
      "user_id": 1,
      "problem_id": 100,
      "language": "C++",
      "status": "Accepted",
      "score": 100,
      "created_at": "2026-05-04T09:30:00Z"
    }
  ]
}
```

---

### GET `/api/v1/contest/{contest_id}/problems`
- **说明**：查看比赛题目列表
- **权限**：需登录且比赛期间内
- **路径参数**：`contest_id` – 比赛 ID
- **请求体**：无
- **返回体**：
```json
{
  "contest_id": 1,
  "problems": [
    {
      "problem_id": 101,
      "title": "A+B Problem",
      "order": 1
    },
    {
      "problem_id": 102,
      "title": "排序",
      "order": 2
    }
  ]
}
```

### GET `/api/v1/contest/{contest_id}/ranking`
- **说明**：获取比赛排名（分数等）
- **权限**：需登录
- **路径参数**：`contest_id` – 比赛 ID
- **请求体**：无
- **返回体**：
```json
{
  "contest_id": 1,
  "rankings": [
    { "rank": 1, "user_id": 2, "username": "user2", "score": 300, "solved": 3 },
    { "rank": 2, "user_id": 1, "username": "alice", "score": 200, "solved": 2 }
  ]
}
```

---

### POST `/api/v1/contest/{contest_id}/submit`
- **说明**：提交代码
- **权限**：需登录
- **路径参数**：`contest_id` – 比赛 ID
- **请求体**：
```json
{
  "problem_id": 100,
  "language": "C++",
  "code": "#include <iostream>\nint main(){...}"
}
```
- **返回体**：
```json
{
  "info": "submission created",
  "submission_id": 2048
}
```

### GET `/api/v1/submission/{submission_id}`
- **说明**：查看某个提交的详细评测结果
- **权限**：需登录（仅可查看自己或比赛内可见）
- **路径参数**：`submission_id` – 提交 ID
- **请求体**：无
- **返回体**：
```json
{
  "id": 2048,
  "user_id": 1,
  "problem_id": 100,
  "contest_id": 1,
  "language": "C++",
  "status": "Accepted",
  "score": 100,
  "time_usage": "12ms",
  "memory_usage": "256KB",
  "details": [
    { "testcase": 1, "result": "AC", "time": "5ms", "memory": "128KB" },
    { "testcase": 2, "result": "AC", "time": "7ms", "memory": "128KB" }
  ],
  "created_at": "2026-05-04T09:30:00Z"
}
```
