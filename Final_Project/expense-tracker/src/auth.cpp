#include "auth.hpp"

#include "utils.hpp"

AuthService::AuthService(const Storage& storage) : storage_(storage) {}

bool AuthService::register_user(const std::string& username, const std::string& password) const {
    if (username.empty() || password.empty()) {
        return false;
    }
    if (storage_.user_exists(username)) {
        return false;
    }
    return storage_.register_user(username, hash_password(password));
}

bool AuthService::login(const std::string& username, const std::string& password) const {
    auto stored = storage_.get_user_hashed_password(username);
    if (!stored.has_value()) {
        return false;
    }
    return stored.value() == hash_password(password);
}
