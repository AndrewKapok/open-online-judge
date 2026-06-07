#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <mutex>
#include "user.h"

/**
 * @brief Default session time-to-live (30 minutes).
 */
constexpr std::chrono::seconds DEFAULT_SESSION_TTL(1800);

/**
 * @brief Structure representing an active user session.
 *
 * Each session is uniquely identified by a session_id and stores
 * essential user context for authenticating subsequent requests.
 */
struct Session {
    std::string session_id;                                     ///< Unique session identifier (generated)
    unsigned int user_id = 0;                                   ///< Associated user ID
    std::string username;                                       ///< Username of the authenticated user
    UserRole role = UserRole::User;                             ///< Role of the authenticated user
    std::chrono::system_clock::time_point created_at;           ///< Timestamp when the session was created
    std::chrono::seconds ttl{DEFAULT_SESSION_TTL};              ///< Time-to-live duration for the session
};

/**
 * @brief Manages user sessions entirely in memory.
 *
 * Provides create / validate / destroy / refresh lifecycle operations.
 * Thread-safe via internal mutex. Sessions are NOT persisted — they
 * are lost on server restart.
 */
class SessionManager {
public:
    /**
     * @brief Create a new session for the specified user.
     * @param user_id  User ID
     * @param username Username
     * @param role     User role
     * @return The generated unique session ID string.
     */
    std::string create_session(unsigned int user_id, const std::string& username, UserRole role);

    /**
     * @brief Validate a session by its ID.
     *
     * Checks that the session exists and has not exceeded its TTL.
     * @param session_id Session ID to validate
     * @return std::optional<Session> containing the session if valid,
     *         or std::nullopt if the session does not exist or has expired.
     */
    std::optional<Session> validate_session(const std::string& session_id);

    /**
     * @brief Destroy (remove) a session — used for logout.
     * @param session_id Session ID to destroy
     */
    void destroy_session(const std::string& session_id);

    /**
     * @brief Refresh a session by resetting its creation time,
     *        effectively extending its TTL.
     * @param session_id Session ID to refresh
     * @return true if the session was successfully refreshed,
     *         false if the session does not exist or has expired.
     */
    bool refresh_session(const std::string& session_id);

private:
    std::unordered_map<std::string, Session> sessions_;  ///< In-memory session storage
    mutable std::mutex mutex_;                           ///< Mutex for thread-safe access

    /**
     * @brief Generate a cryptographically-inspired random session ID.
     * @return A hex-encoded string of random bytes.
     */
    std::string generate_session_id();

    /**
     * @brief Check whether a session has expired relative to the current time.
     * @param session The session to check
     * @return true if the session has expired, false otherwise
     */
    bool is_expired(const Session& session) const;
};