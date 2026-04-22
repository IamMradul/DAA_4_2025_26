#include "utils.hpp"

#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

namespace {
std::string to_lower_hex(std::uint64_t value) {
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << value;
    return out.str();
}
}

int parse_date_to_key(const std::string& dd_mm_yyyy) {
    if (dd_mm_yyyy.size() != 10 || dd_mm_yyyy[2] != '-' || dd_mm_yyyy[5] != '-') {
        throw std::runtime_error("Date must be DD-MM-YYYY");
    }

    int day = std::stoi(dd_mm_yyyy.substr(0, 2));
    int month = std::stoi(dd_mm_yyyy.substr(3, 2));
    int year = std::stoi(dd_mm_yyyy.substr(6, 4));

    if (month < 1 || month > 12 || day < 1 || day > 31 || year < 1900) {
        throw std::runtime_error("Invalid date value");
    }

    return (year * 10000) + (month * 100) + day;
}

std::string key_to_date_string(int key) {
    int year = key / 10000;
    int month = (key / 100) % 100;
    int day = key % 100;

    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << day << "-"
        << std::setfill('0') << std::setw(2) << month << "-"
        << std::setfill('0') << std::setw(4) << year;
    return out.str();
}

std::optional<MonthYear> parse_month_year(const std::string& mm_yyyy) {
    if (mm_yyyy.empty()) {
        return std::nullopt;
    }
    if (mm_yyyy.size() != 7 || mm_yyyy[2] != '-') {
        return std::nullopt;
    }

    int month = std::stoi(mm_yyyy.substr(0, 2));
    int year = std::stoi(mm_yyyy.substr(3, 4));
    if (month < 1 || month > 12 || year < 1900) {
        return std::nullopt;
    }

    return MonthYear{month, year};
}

std::string hash_password(const std::string& plain) {
    // Deterministic app-level hash for local use.
    const std::string salted = "expense-tracker-cpp::" + plain;
    std::uint64_t hash = 1469598103934665603ULL;
    for (unsigned char c : salted) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return to_lower_hex(hash);
}

std::string gen_id() {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<std::uint64_t> dist;
    return to_lower_hex(dist(rng)) + to_lower_hex(dist(rng));
}

std::string html_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string url_encode(const std::string& value) {
    std::ostringstream out;
    out << std::hex << std::uppercase;

    for (unsigned char c : value) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out << c;
        } else if (c == ' ') {
            out << '+';
        } else {
            out << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
    }

    return out.str();
}
