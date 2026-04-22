#pragma once

#include "storage.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

struct MonthlyReport {
    int month{};
    int year{};
    double total{};
    std::vector<std::pair<std::string, double>> top_categories;
    std::optional<std::pair<int, double>> highest_day;
};

class UserSession {
public:
    UserSession(std::string username, const Storage& storage);

    void load();
    void save() const;

    const std::string& username() const;
    const std::optional<double>& budget() const;
    const std::vector<Expense>& expenses() const;

    void set_budget(double amount);
    void add_expense(const Expense& expense);
    bool delete_expense(const std::string& expense_id);

    std::vector<Expense> filtered_expenses(const std::string& category) const;
    MonthlyReport month_report(int month, int year) const;

private:
    std::string username_;
    const Storage& storage_;
    UserData data_;
};
