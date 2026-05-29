# 前端模块文档

## 概述

前端模块是在线评测系统的用户界面层，采用单页应用架构，与后端 cpp-httplib 服务通过 RESTful API 进行交互。前端负责页面渲染、用户交互和缓存管理。

## 路由与页面设计

### 根路径 `/`

- **未登录状态**：自动 302 重定向到 `/login`
- **已登录状态**：返回用户 Dashboard 页面

### 登录页 `/login`

- 提供用户登录和注册表单
- 登录成功后 302 重定向到首页 `/`
- 对接后端 API：
  - `/api/v1/register` - 用户注册
  - `/api/v1/login` - 用户登录

### Dashboard（首页）

- 显示当前登录用户名
- 按状态分组展示比赛：
  - 已开始的比赛
  - 即将开始的比赛
  - 已结束的比赛
- 展示比赛相关信息：
  - 距开始时间
  - 已开始时长
  - 结束时间
- 点击比赛可进入对应比赛页面
- 顶部导航栏提供：
  - 比赛列表入口
  - 用户设置入口

### 比赛路由 `/contest`

#### 比赛列表页
- 按时间倒序展示比赛
- 分组显示：已开始 > 即将开始 > 已结束

#### 比赛详情页 `/contest/{contest_id}`
- 显示比赛基本信息：
  - 比赛开始时间
  - 赛制说明
  - 题目数量
  - 其他比赛概况

#### 比赛子页面
- **题目详情** `/contest/{id}/problem/{problem_id}`
  - 题目描述
  - 提交入口
  - 题目限制说明

- **排行榜** `/contest/{id}/rank`
  - 实时排名
  - 用户得分情况

- **提交记录** `/contest/{id}/submissions`
  - 所有用户的提交历史
  - 评测结果展示

### 用户设置页
- 修改密码功能
- 更新个人信息
- 对接后端 `/api/v1/user/profile/update` API

## API 接口对接

### 用户认证
- 注册：`POST /api/v1/register`
- 登录：`POST /api/v1/login`
- 登出：`POST /api/v1/logout`

### 数据获取
- 比赛列表：`GET /api/v1/contests`
- 题目详情：`GET /api/v1/contest/{id}/problem/{problem_id}`
- 提交记录：`GET /api/v1/contest/{id}/submissions`
- 排行榜：`GET /api/v1/contest/{id}/rank`

### 用户操作
- 代码提交：`POST /api/v1/contest/{id}/submit`
- 个人信息更新：`PUT /api/v1/user/profile/update`

## 技术特性

- 响应式设计，支持多种设备
- 前后端分离架构
- Session-based 认证机制（Cookie）
- 实时数据更新（排行榜、提交状态等）
