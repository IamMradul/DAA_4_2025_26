#pragma once

#include "storage.hpp"

#include <string>

class AuthService {
public:
    explicit AuthService(const Storage& storage);

    bool register_user(const std::string& username, const std::string& password) const;
    bool login(const std::string& username, const std::string& password) const;

private:
    const Storage& storage_;
};
