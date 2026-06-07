#include "session.h"
#include <random>
#include <iomanip>
#include <sstream>

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

std::string SessionManager::generate_session_id()
{
    static constexpr std::size_t ID_LENGTH = 32;  // 32 hex chars → 128 bit

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dist;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < ID_LENGTH / sizeof(std::uint64_t) * 2; ++i) {
        oss << std::setw(sizeof(std::uint64_t) * 2) << dist(gen);
    }
    auto id = oss.str();
    id.resize(ID_LENGTH);  // trim in case of excess
    return id;
}

bool SessionManager::is_expired(const Session& session) const
{
    auto now = std::chrono::system_clock::now();
    return (now - session.created_at) > session.ttl;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string SessionManager::create_session(unsigned int user_id,
                                           const std::string& username,
                                           UserRole role)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Session session;
    session.session_id = generate_session_id();
    session.user_id    = user_id;
    session.username   = username;
    session.role       = role;
    session.created_at = std::chrono::system_clock::now();
    session.ttl        = DEFAULT_SESSION_TTL;

    sessions_[session.session_id] = std::move(session);
    return sessions_[session.session_id].session_id;
}

std::optional<Session> SessionManager::validate_session(const std::string& session_id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return std::nullopt;
    }

    if (is_expired(it->second)) {
        sessions_.erase(it);
        return std::nullopt;
    }

    return it->second;
}

void SessionManager::destroy_session(const std::string& session_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session_id);
}

bool SessionManager::refresh_session(const std::string& session_id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }

    if (is_expired(it->second)) {
        sessions_.erase(it);
        return false;
    }

    // Reset creation time to now → effectively extends TTL
    it->second.created_at = std::chrono::system_clock::now();
    return true;
}