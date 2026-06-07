#pragma once
#include <mutex>
#include <string>

#include "json/single_include/nlohmann/json.hpp"
#include "session.h"
#include "user.h"

using nlohmann::json;

/**
 * @file auth.h
 * @brief Authentication manager — user registration, login, and logout.
 *
 * Dependencies:
 *   - hash.h / hash.cpp   (password salting and SHA-256 hashing)
 *   - SessionManager      (in-memory session lifecycle)
 *   - JSON storage        (users persisted in data/users.json)
 */

/**
 * @brief Result of an authentication operation (register / login).
 */
struct AuthResult
{
    bool success;           ///< Whether the operation succeeded
    std::string message;    ///< Human-readable status message
    std::string session_id; ///< Session ID (non-empty only on successful login)
};

/**
 * @brief Manages user authentication with JSON-backed storage.
 *
 * Responsibilities:
 *   - Register new users with salted password hashes
 *   - Authenticate existing users and create sessions
 *   - Destroy sessions on logout
 *
 * User data is persisted in a configurable JSON file (default: "data/users.json").
 * Thread-safe via internal mutex.
 */
class AuthManager
{
public:
    /**
     * @brief Construct an AuthManager.
     *
     * @param session_manager  Reference to the application's SessionManager
     * @param data_file        Path to the users JSON data file
     *                         (default: "data/users.json")
     */
    explicit AuthManager(SessionManager& session_manager, const std::string& data_file = "data/users.json");

    // ── Public API ───────────────────────────────────────────────────────

    /**
     * @brief Register a new user account.
     *
     * Validation checks:
     *   - Username: 3–32 alphanumeric characters (underscore allowed)
     *   - Password: 6–128 characters
     *   - Username must not already exist
     *
     * On success the user is persisted and a new session is created.
     *
     * @param username  Desired login name
     * @param password  Plaintext password
     * @return AuthResult — success contains the new session_id;
     *         failure contains an error message.
     */
    AuthResult register_user(const std::string& username, const std::string& password);

    /**
     * @brief Authenticate an existing user.
     *
     * Verifies the salted password hash and creates a new session
     * on success.
     *
     * @param username  Login name
     * @param password  Plaintext password
     * @return AuthResult — success contains a session_id;
     *         failure contains an error message.
     */
    AuthResult login_user(const std::string& username, const std::string& password);

    /**
     * @brief Log out a user by destroying their session.
     *
     * @param session_id  Session ID to destroy
     */
    void logout_user(const std::string& session_id);

private:
    SessionManager& session_manager_; ///< Session lifecycle manager
    std::string data_file_;           ///< Path to users.json
    mutable std::mutex mutex_;        ///< Thread-safety guard

    // ── Internal helpers ─────────────────────────────────────────────────

    /**
     * @brief Load the users array from the JSON data file.
     *
     * If the file does not exist or is malformed, an empty array
     * is returned.
     *
     * @return JSON array of user objects
     */
    json load_users() const;

    /**
     * @brief Persist the users array to the JSON data file.
     *
     * Creates the parent directory if it does not exist.
     *
     * @param users  JSON array of user objects
     */
    void save_users(const json& users) const;

    /**
     * @brief Compute the next available user ID.
     *
     * @param users  JSON array of existing users
     * @return The maximum existing user_id + 1 (starts at 1)
     */
    static unsigned int get_next_user_id(const json& users);

    /**
     * @brief Validate a username string.
     *
     * Rules: 3–32 characters, only alphanumeric and underscore.
     *
     * @param username  Username to validate
     * @return Empty string if valid, otherwise an error description.
     */
    static std::string validate_username(const std::string& username);

    /**
     * @brief Validate a password string.
     *
     * Rules: 6–128 characters.
     *
     * @param password  Password to validate
     * @return Empty string if valid, otherwise an error description.
     */
    static std::string validate_password(const std::string& password);

    /**
     * @brief Check whether a username already exists in the given users array.
     *
     * @param users     JSON array of user objects
     * @param username  Username to look for
     * @return true if the username already exists
     */
    static bool username_exists(const json& users, const std::string& username);
};