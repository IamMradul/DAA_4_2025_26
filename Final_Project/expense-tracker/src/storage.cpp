#include "storage.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {
using nlohmann::json;

json expense_to_json(const Expense& e) {
    return json{{"id", e.id}, {"date_key", e.date_key}, {"category", e.category}, {"amount", e.amount}, {"note", e.note}};
}

Expense expense_from_json(const json& j) {
    return Expense{
        j.at("id").get<std::string>(),
        j.at("date_key").get<int>(),
        j.at("category").get<std::string>(),
        j.at("amount").get<double>(),
        j.value("note", "")
    };
}

json read_json_file(const std::filesystem::path& path, const json& fallback) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return fallback;
    }
    json data;
    in >> data;
    return data;
}

void write_json_file(const std::filesystem::path& path, const json& data) {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + path.string());
    }
    out << data.dump(2);
}
}

Storage::Storage(std::filesystem::path data_dir)
    : data_dir_(std::move(data_dir)), users_file_(data_dir_ / "users.json") {
    ensure_data_files();
}

void Storage::ensure_data_files() const {
    std::filesystem::create_directories(data_dir_);
    if (!std::filesystem::exists(users_file_)) {
        write_json_file(users_file_, nlohmann::json::object());
    }
}

bool Storage::user_exists(const std::string& username) const {
    auto users = read_json_file(users_file_, nlohmann::json::object());
    return users.contains(username);
}

std::optional<std::string> Storage::get_user_hashed_password(const std::string& username) const {
    auto users = read_json_file(users_file_, nlohmann::json::object());
    if (!users.contains(username)) {
        return std::nullopt;
    }
    return users.at(username).get<std::string>();
}

bool Storage::register_user(const std::string& username, const std::string& hashed_password) const {
    auto users = read_json_file(users_file_, nlohmann::json::object());
    if (users.contains(username)) {
        return false;
    }

    users[username] = hashed_password;
    write_json_file(users_file_, users);

    const auto user_file = data_dir_ / (username + ".json");
    if (!std::filesystem::exists(user_file)) {
        nlohmann::json initial{{"budget", nullptr}, {"expenses", nlohmann::json::array()}};
        write_json_file(user_file, initial);
    }

    return true;
}

UserData Storage::load_user_data(const std::string& username) const {
    const auto user_file = data_dir_ / (username + ".json");
    if (!std::filesystem::exists(user_file)) {
        return UserData{};
    }

    auto data = read_json_file(user_file, nlohmann::json::object());

    UserData out;
    if (data.contains("budget") && !data.at("budget").is_null()) {
        out.budget = data.at("budget").get<double>();
    }

    if (data.contains("expenses") && data.at("expenses").is_array()) {
        for (const auto& item : data.at("expenses")) {
            out.expenses.push_back(expense_from_json(item));
        }
    }

    return out;
}

void Storage::save_user_data(const std::string& username, const UserData& data) const {
    nlohmann::json payload;
    if (data.budget.has_value()) {
        payload["budget"] = data.budget.value();
    } else {
        payload["budget"] = nullptr;
    }

    payload["expenses"] = nlohmann::json::array();
    for (const auto& e : data.expenses) {
        payload["expenses"].push_back(expense_to_json(e));
    }

    write_json_file(data_dir_ / (username + ".json"), payload);
}
