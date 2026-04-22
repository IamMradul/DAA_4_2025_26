#pragma once

#include <string>

struct Expense {
    std::string id;
    int date_key{};
    std::string category;
    double amount{};
    std::string note;
};
