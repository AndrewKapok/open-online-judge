#include "user.h"

User::User(unsigned int id, const std::string& username, const std::string& password, UserRole role)
    : user_id_(id), username_(username), password_(password), role_(role)
{
}

unsigned int User::get_user_id() const
{
    return user_id_;
}

const std::string& User::get_username() const
{
    return username_;
}

const std::string& User::get_password() const
{
    return password_;
}

UserRole User::get_role() const
{
    return role_;
}

void User::set_username(const std::string& username)
{
    username_ = username;
}

void User::set_password(const std::string& password)
{
    password_ = password;
}

void User::set_role(UserRole role)
{
    role_ = role;
}