#include "auth.h"
#include "hash.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>

// ============================================================================
// Construction
// ============================================================================

AuthManager::AuthManager(SessionManager& session_manager,
                         const std::string& data_file)
    : session_manager_(session_manager)
    , data_file_(data_file)
{
}

// ============================================================================
// Public API
// ============================================================================

AuthResult AuthManager::register_user(const std::string& username,
                                      const std::string& password)
{
    // ── Validate inputs ──────────────────────────────────────────────
    auto name_err = validate_username(username);
    if (!name_err.empty()) {
        return {false, std::move(name_err), {}};
    }

    auto pass_err = validate_password(password);
    if (!pass_err.empty()) {
        return {false, std::move(pass_err), {}};
    }

    // ── Thread-safe read-modify-write ────────────────────────────────
    std::lock_guard<std::mutex> lock(mutex_);

    auto users = load_users();

    if (username_exists(users, username)) {
        return {false, "用户名已存在", {}};
    }

    // Hash password and build user record
    auto salt   = generate_salt();
    auto salted = hash_password(password, salt);

    auto id = get_next_user_id(users);

    json user_entry = {
        {"user_id",  id},
        {"username", username},
        {"password", salted},
        {"role",     static_cast<int>(UserRole::User)}
    };
    users.push_back(std::move(user_entry));
    save_users(users);

    // Create a session for the newly registered user
    auto session_id = session_manager_.create_session(id, username,
                                                      UserRole::User);

    return {true, "注册成功", std::move(session_id)};
}

AuthResult AuthManager::login_user(const std::string& username,
                                   const std::string& password)
{
    // ── Basic validation ─────────────────────────────────────────────
    if (username.empty() || password.empty()) {
        return {false, "用户名和密码不能为空", {}};
    }

    // ── Thread-safe lookup ───────────────────────────────────────────
    std::lock_guard<std::mutex> lock(mutex_);

    auto users = load_users();

    // Find user by username
    json* found = nullptr;
    for (auto& u : users) {
        if (u["username"] == username) {
            found = &u;
            break;
        }
    }

    if (!found) {
        return {false, "用户名或密码错误", {}};
    }

    // Verify password
    auto stored_hash = (*found)["password"].get<std::string>();
    if (!verify_password(password, stored_hash)) {
        return {false, "用户名或密码错误", {}};
    }

    auto user_id   = (*found)["user_id"].get<unsigned int>();
    auto role_int  = (*found)["role"].get<int>();
    auto role      = static_cast<UserRole>(role_int);

    // Create session
    auto session_id = session_manager_.create_session(user_id, username, role);

    return {true, "登录成功", std::move(session_id)};
}

void AuthManager::logout_user(const std::string& session_id)
{
    session_manager_.destroy_session(session_id);
}

// ============================================================================
// Internal helpers
// ============================================================================

json AuthManager::load_users() const
{
    std::ifstream file(data_file_);
    if (!file.is_open()) {
        // File doesn't exist yet — return empty array
        return json::array();
    }

    try {
        json data;
        file >> data;
        return data.is_array() ? data : json::array();
    } catch (const json::exception&) {
        // Malformed file — start fresh
        return json::array();
    }
}

void AuthManager::save_users(const json& users) const
{
    // Ensure the parent directory exists
    auto sep = data_file_.find_last_of("/\\");
    if (sep != std::string::npos) {
        auto dir = data_file_.substr(0, sep);
        // Use platform-independent approach: attempt to create dir
        int ret = 0;
        std::string mkdir_cmd;
#ifdef _WIN32
        mkdir_cmd = "if not exist \"" + dir + "\" mkdir \"" + dir + "\"";
#else
        mkdir_cmd = "mkdir -p \"" + dir + "\"";
#endif
        ret = std::system(mkdir_cmd.c_str());
        (void)ret; // ignore failure — the write below will catch errors
    }

    std::ofstream file(data_file_);
    if (!file.is_open()) {
        std::cerr << "[AuthManager] 无法写入数据文件: " << data_file_
                  << std::endl;
        return;
    }

    file << users.dump(4) << std::endl;
}

unsigned int AuthManager::get_next_user_id(const json& users)
{
    unsigned int max_id = 0;
    for (const auto& u : users) {
        auto uid = u["user_id"].get<unsigned int>();
        if (uid > max_id) {
            max_id = uid;
        }
    }
    return max_id + 1;
}

std::string AuthManager::validate_username(const std::string& username)
{
    if (username.empty()) {
        return "用户名不能为空";
    }

    if (username.length() < 3 || username.length() > 32) {
        return "用户名长度必须在 3–32 个字符之间";
    }

    // Alphanumeric and underscore only
    for (auto ch : username) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            return "用户名只能包含字母、数字和下划线";
        }
    }

    return {};
}

std::string AuthManager::validate_password(const std::string& password)
{
    if (password.empty()) {
        return "密码不能为空";
    }

    if (password.length() < 6 || password.length() > 128) {
        return "密码长度必须在 6–128 个字符之间";
    }

    return {};
}

bool AuthManager::username_exists(const json& users,
                                  const std::string& username)
{
    return std::any_of(users.begin(), users.end(),
                       [&](const json& u) {
                           return u["username"] == username;
                       });
}