#pragma once

#include <optional>
#include <string>

struct MonthYear {
    int month{};
    int year{};
};

int parse_date_to_key(const std::string& dd_mm_yyyy);
std::string key_to_date_string(int key);
std::optional<MonthYear> parse_month_year(const std::string& mm_yyyy);
std::string hash_password(const std::string& plain);
std::string gen_id();
std::string html_escape(const std::string& value);
std::string url_encode(const std::string& value);
