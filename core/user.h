#pragma once
#include <string>

/**
 * @brief Enumeration of user roles in the system.
 */
enum class UserRole {
    Admin = 0,  ///< Administrator with full system access
    User = 1    ///< Regular user with limited access
};

/**
 * @brief Represents a user in the online judge system.
 *
 * Stores core user information including credentials and role.
 * Passwords are expected to be stored as salted hashes externally.
 */
class User {
private:
    unsigned int user_id_;      ///< Unique user identifier
    std::string username_;      ///< Login username
    std::string password_;      ///< Password (salted hash)
    UserRole role_;             ///< User role (Admin or User)

public:
    /**
     * @brief Default constructor.
     */
    User() = default;

    /**
     * @brief Construct a new User with full details.
     * @param id       Unique user ID
     * @param username Login username
     * @param password Password (salted hash)
     * @param role     User role
     */
    User(unsigned int id, const std::string& username, const std::string& password, UserRole role);

    // ── Getters ──────────────────────────────────────────────

    /** @brief Get the user ID. */
    unsigned int get_user_id() const;

    /** @brief Get the username. */
    const std::string& get_username() const;

    /** @brief Get the password hash. */
    const std::string& get_password() const;

    /** @brief Get the user role. */
    UserRole get_role() const;

    // ── Setters ──────────────────────────────────────────────

    /** @brief Set the username. */
    void set_username(const std::string& username);

    /** @brief Set the password hash. */
    void set_password(const std::string& password);

    /** @brief Set the user role. */
    void set_role(UserRole role);
};