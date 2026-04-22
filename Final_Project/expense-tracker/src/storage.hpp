#pragma once

#include "models.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct UserData {
    std::optional<double> budget;
    std::vector<Expense> expenses;
};

class Storage {
public:
    explicit Storage(std::filesystem::path data_dir);

    void ensure_data_files() const;
    bool user_exists(const std::string& username) const;
    std::optional<std::string> get_user_hashed_password(const std::string& username) const;
    bool register_user(const std::string& username, const std::string& hashed_password) const;
    UserData load_user_data(const std::string& username) const;
    void save_user_data(const std::string& username, const UserData& data) const;

private:
    std::filesystem::path data_dir_;
    std::filesystem::path users_file_;
};
