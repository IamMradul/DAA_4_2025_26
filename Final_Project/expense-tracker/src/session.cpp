#include "session.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <unordered_map>

namespace {
bool same_month_year(int date_key, int month, int year) {
    const int y = date_key / 10000;
    const int m = (date_key / 100) % 100;
    return y == year && m == month;
}

bool equals_ignore_case(std::string a, std::string b) {
    std::transform(a.begin(), a.end(), a.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::transform(b.begin(), b.end(), b.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return a == b;
}
}

UserSession::UserSession(std::string username, const Storage& storage)
    : username_(std::move(username)), storage_(storage) {}

void UserSession::load() {
    data_ = storage_.load_user_data(username_);
}

void UserSession::save() const {
    storage_.save_user_data(username_, data_);
}

const std::string& UserSession::username() const {
    return username_;
}

const std::optional<double>& UserSession::budget() const {
    return data_.budget;
}

const std::vector<Expense>& UserSession::expenses() const {
    return data_.expenses;
}

void UserSession::set_budget(double amount) {
    data_.budget = amount;
}

void UserSession::add_expense(const Expense& expense) {
    data_.expenses.push_back(expense);
    std::sort(data_.expenses.begin(), data_.expenses.end(), [](const Expense& a, const Expense& b) {
        if (a.date_key != b.date_key) {
            return a.date_key < b.date_key;
        }
        return a.id < b.id;
    });
}

bool UserSession::delete_expense(const std::string& expense_id) {
    const auto it = std::remove_if(data_.expenses.begin(), data_.expenses.end(), [&](const Expense& e) {
        return e.id == expense_id;
    });

    if (it == data_.expenses.end()) {
        return false;
    }

    data_.expenses.erase(it, data_.expenses.end());
    return true;
}

std::vector<Expense> UserSession::filtered_expenses(const std::string& category) const {
    if (category.empty()) {
        return data_.expenses;
    }

    std::vector<Expense> out;
    for (const auto& e : data_.expenses) {
        if (equals_ignore_case(e.category, category)) {
            out.push_back(e);
        }
    }
    return out;
}

MonthlyReport UserSession::month_report(int month, int year) const {
    MonthlyReport report;
    report.month = month;
    report.year = year;

    std::unordered_map<std::string, double> category_sums;
    std::unordered_map<int, double> day_sums;

    for (const auto& e : data_.expenses) {
        if (!same_month_year(e.date_key, month, year)) {
            continue;
        }

        report.total += e.amount;
        category_sums[e.category] += e.amount;
        day_sums[e.date_key] += e.amount;
    }

    report.top_categories.reserve(category_sums.size());
    for (const auto& [category, amount] : category_sums) {
        report.top_categories.emplace_back(category, amount);
    }

    std::sort(report.top_categories.begin(), report.top_categories.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    if (report.top_categories.size() > 3) {
        report.top_categories.resize(3);
    }

    double best_amount = -1.0;
    int best_day_key = 0;
    for (const auto& [key, amount] : day_sums) {
        if (amount > best_amount) {
            best_amount = amount;
            best_day_key = key;
        }
    }

    if (best_amount >= 0.0) {
        report.highest_day = std::make_pair(best_day_key, best_amount);
    }

    return report;
}
