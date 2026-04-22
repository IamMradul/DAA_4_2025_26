#include "auth.hpp"
#include "session.hpp"
#include "utils.hpp"

#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <httplib.h>

namespace {

std::string format_money(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

std::string get_cookie(const httplib::Request& req, const std::string& name) {
    auto it = req.headers.find("Cookie");
    if (it == req.headers.end()) {
        return "";
    }

    std::string cookie = it->second;
    std::string key = name + "=";

    size_t pos = 0;
    while (pos < cookie.size()) {
        size_t end = cookie.find(';', pos);
        if (end == std::string::npos) {
            end = cookie.size();
        }

        std::string token = cookie.substr(pos, end - pos);
        while (!token.empty() && token.front() == ' ') {
            token.erase(token.begin());
        }

        if (token.rfind(key, 0) == 0) {
            return token.substr(key.size());
        }

        pos = end + 1;
    }

    return "";
}

std::string alert_html(const httplib::Request& req) {
    if (!req.has_param("msg") || !req.has_param("type")) {
        return "";
    }

    const auto type = req.get_param_value("type");
    const auto msg = html_escape(req.get_param_value("msg"));
    if (type != "success" && type != "warning" && type != "error") {
        return "";
    }

    return "<div class=\"alert alert-" + type + "\">" + msg + "</div>";
}

std::string page_frame(const std::string& title, const std::optional<std::string>& username, const std::string& alerts, const std::string& content) {
    std::ostringstream out;
    out << "<!doctype html><html lang=\"en\"><head>"
        << "<meta charset=\"utf-8\"/>"
        << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>"
        << "<title>" << html_escape(title) << "</title>"
        << "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\"/>"
        << "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin/>"
        << "<link href=\"https://fonts.googleapis.com/css2?family=Manrope:wght@400;500;600;700&display=swap\" rel=\"stylesheet\"/>"
        << "<link rel=\"stylesheet\" href=\"/static/styles.css\"/>"
        << "</head><body>"
        << "<div class=\"bg-shape bg-shape-1\"></div><div class=\"bg-shape bg-shape-2\"></div>"
        << "<header class=\"topbar\"><div class=\"topbar-inner\">"
        << "<a class=\"brand\" href=\"" << (username.has_value() ? "/dashboard" : "/login") << "\">Expense Tracker</a>";

    if (username.has_value()) {
        out << "<nav><span class=\"muted\">" << html_escape(username.value())
            << "</span><a href=\"/logout\">Logout</a></nav>";
    }

    out << "</div></header><main class=\"container\">"
        << alerts
        << content
        << "</main></body></html>";

    return out.str();
}

std::string login_page(const httplib::Request& req) {
    const std::string body =
        "<section class=\"card auth-card\">"
        "<h1>Welcome back</h1>"
        "<p class=\"muted\">Sign in to manage your expenses.</p>"
        "<form method=\"post\" class=\"form-grid\">"
        "<label>Username<input type=\"text\" name=\"username\" required/></label>"
        "<label>Password<input type=\"password\" name=\"password\" required/></label>"
        "<button class=\"btn\" type=\"submit\">Login</button>"
        "</form>"
        "<p class=\"muted small\">New here? <a href=\"/register\">Create an account</a></p>"
        "</section>";

    return page_frame("Login", std::nullopt, alert_html(req), body);
}

std::string register_page(const httplib::Request& req) {
    const std::string body =
        "<section class=\"card auth-card\">"
        "<h1>Create account</h1>"
        "<p class=\"muted\">Start tracking with a clean workflow.</p>"
        "<form method=\"post\" class=\"form-grid\">"
        "<label>Username<input type=\"text\" name=\"username\" required/></label>"
        "<label>Password<input type=\"password\" name=\"password\" required/></label>"
        "<button class=\"btn\" type=\"submit\">Register</button>"
        "</form>"
        "<p class=\"muted small\">Already have an account? <a href=\"/login\">Login</a></p>"
        "</section>";

    return page_frame("Register", std::nullopt, alert_html(req), body);
}

std::string dashboard_page(const httplib::Request& req, const UserSession& s, const std::vector<Expense>& expenses, const MonthlyReport& report) {
    const auto selected_category = req.has_param("category") ? req.get_param_value("category") : "";
    const auto selected_month = req.has_param("month") ? req.get_param_value("month") : "";

    std::ostringstream body;
    body << "<section class=\"grid two\">";

    body << "<div class=\"card\"><h2>Add Expense</h2>"
         << "<form method=\"post\" action=\"/expenses/add\" class=\"form-grid\">"
         << "<label>Date<input type=\"text\" name=\"date\" placeholder=\"DD-MM-YYYY\" required/></label>"
         << "<label>Category<input type=\"text\" name=\"category\" placeholder=\"Food, Travel, Bills\" required/></label>"
         << "<label>Amount<input type=\"number\" step=\"0.01\" min=\"0.01\" name=\"amount\" required/></label>"
         << "<label>Note<input type=\"text\" name=\"note\" placeholder=\"Optional\"/></label>"
         << "<button class=\"btn\" type=\"submit\">Add</button>"
         << "</form></div>";

    body << "<div class=\"card\"><h2>Budget</h2><p class=\"muted\">Current: ";
    if (s.budget().has_value()) {
        body << "Rs " << format_money(s.budget().value());
    } else {
        body << "Not set";
    }
    body << "</p>"
         << "<form method=\"post\" action=\"/budget\" class=\"inline-form\">"
         << "<input type=\"number\" step=\"0.01\" min=\"0.01\" name=\"budget\" placeholder=\"Monthly budget\" required/>"
         << "<button class=\"btn secondary\" type=\"submit\">Update</button></form>";

    body << "<h3>Monthly Report (" << std::setfill('0') << std::setw(2) << report.month << "-" << report.year << ")</h3>"
         << "<p>Total: <strong>Rs " << format_money(report.total) << "</strong></p>";

    if (s.budget().has_value()) {
        const double remaining = s.budget().value() - report.total;
        if (remaining < 0) {
            body << "<p class=\"text-danger\">Over budget by Rs " << format_money(-remaining) << "</p>";
        } else {
            body << "<p class=\"text-success\">Remaining Rs " << format_money(remaining) << "</p>";
        }
    }

    if (!report.top_categories.empty()) {
        body << "<h4>Top Categories</h4><ul class=\"simple-list\">";
        for (const auto& [cat, amt] : report.top_categories) {
            body << "<li>" << html_escape(cat) << " <span>Rs " << format_money(amt) << "</span></li>";
        }
        body << "</ul>";
    }

    if (report.highest_day.has_value()) {
        body << "<p class=\"muted\">Highest day: " << key_to_date_string(report.highest_day->first)
             << " (Rs " << format_money(report.highest_day->second) << ")</p>";
    }

    body << "</div></section>";

    body << "<section class=\"card\"><div class=\"table-head\"><h2>Expenses</h2><div class=\"actions\">"
         << "<form method=\"get\" action=\"/dashboard\" class=\"inline-form\">"
         << "<input type=\"text\" name=\"category\" value=\"" << html_escape(selected_category) << "\" placeholder=\"Filter category\"/>"
         << "<input type=\"text\" name=\"month\" value=\"" << html_escape(selected_month) << "\" placeholder=\"MM-YYYY\"/>"
         << "<button class=\"btn ghost\" type=\"submit\">Apply</button></form>"
         << "<a class=\"btn secondary\" href=\"/export\">Export CSV</a></div></div>";

    body << "<div class=\"table-wrap\"><table><thead><tr>"
         << "<th>ID</th><th>Date</th><th>Category</th><th>Amount</th><th>Note</th><th>Action</th>"
         << "</tr></thead><tbody>";

    if (expenses.empty()) {
        body << "<tr><td colspan=\"6\" class=\"muted center\">No expenses found.</td></tr>";
    } else {
        for (const auto& e : expenses) {
            const std::string short_id = e.id.size() > 8 ? e.id.substr(0, 8) + "..." : e.id;
            body << "<tr><td>" << html_escape(short_id) << "</td>"
                 << "<td>" << key_to_date_string(e.date_key) << "</td>"
                 << "<td>" << html_escape(e.category) << "</td>"
                 << "<td>Rs " << format_money(e.amount) << "</td>"
                 << "<td>" << html_escape(e.note) << "</td>"
                 << "<td><form method=\"post\" action=\"/expenses/delete\">"
                 << "<input type=\"hidden\" name=\"expense_id\" value=\"" << html_escape(e.id) << "\"/>"
                 << "<button class=\"btn danger\" type=\"submit\">Delete</button>"
                 << "</form></td></tr>";
        }
    }

    body << "</tbody></table></div></section>";

    return page_frame("Dashboard", s.username(), alert_html(req), body.str());
}

std::string make_redirect_with_msg(const std::string& path, const std::string& type, const std::string& msg) {
    return path + "?type=" + url_encode(type) + "&msg=" + url_encode(msg);
}

bool is_valid_username(const std::string& username) {
    if (username.empty()) {
        return false;
    }
    for (char c : username) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

} // namespace

int main() {
    const auto base = std::filesystem::current_path();
    Storage storage(base / "data");
    AuthService auth(storage);

    std::unordered_map<std::string, std::string> token_to_user;

    httplib::Server app;

    app.Get("/static/styles.css", [&](const httplib::Request&, httplib::Response& res) {
        std::ifstream css_file(base / "static" / "styles.css");
        if (!css_file.is_open()) {
            res.status = 404;
            res.set_content("/* styles not found */", "text/css");
            return;
        }
        std::ostringstream buf;
        buf << css_file.rdbuf();
        res.set_content(buf.str(), "text/css");
    });

    app.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
        (void)req;
        const auto sid = get_cookie(req, "sid");
        if (!sid.empty() && token_to_user.find(sid) != token_to_user.end()) {
            res.set_redirect("/dashboard");
            return;
        }
        res.set_redirect("/login");
    });

    app.Get("/register", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(register_page(req), "text/html");
    });

    app.Post("/register", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string username = req.has_param("username") ? req.get_param_value("username") : "";
        const std::string password = req.has_param("password") ? req.get_param_value("password") : "";

        if (!is_valid_username(username) || password.empty()) {
            res.set_redirect(make_redirect_with_msg("/register", "error", "Valid username and password are required.").c_str());
            return;
        }

        if (auth.register_user(username, password)) {
            res.set_redirect(make_redirect_with_msg("/login", "success", "Registration successful. Please login.").c_str());
            return;
        }

        res.set_redirect(make_redirect_with_msg("/register", "warning", "Username already exists.").c_str());
    });

    app.Get("/login", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(login_page(req), "text/html");
    });

    app.Post("/login", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string username = req.has_param("username") ? req.get_param_value("username") : "";
        const std::string password = req.has_param("password") ? req.get_param_value("password") : "";

        if (!auth.login(username, password)) {
            res.set_redirect(make_redirect_with_msg("/login", "error", "Invalid credentials.").c_str());
            return;
        }

        const std::string sid = gen_id();
        token_to_user[sid] = username;
        res.set_header("Set-Cookie", ("sid=" + sid + "; Path=/; HttpOnly").c_str());
        res.set_redirect(make_redirect_with_msg("/dashboard", "success", "Login successful.").c_str());
    });

    app.Get("/logout", [&](const httplib::Request& req, httplib::Response& res) {
        const std::string sid = get_cookie(req, "sid");
        if (!sid.empty()) {
            token_to_user.erase(sid);
        }
        res.set_header("Set-Cookie", "sid=; Path=/; Max-Age=0; HttpOnly");
        res.set_redirect(make_redirect_with_msg("/login", "success", "Logged out.").c_str());
    });

    auto get_user_from_session = [&](const httplib::Request& req) -> std::optional<std::string> {
        const std::string sid = get_cookie(req, "sid");
        if (sid.empty()) {
            return std::nullopt;
        }

        auto it = token_to_user.find(sid);
        if (it == token_to_user.end()) {
            return std::nullopt;
        }

        return it->second;
    };

    app.Get("/dashboard", [&](const httplib::Request& req, httplib::Response& res) {
        auto username = get_user_from_session(req);
        if (!username.has_value()) {
            res.set_redirect("/login");
            return;
        }

        UserSession s(username.value(), storage);
        s.load();

        std::vector<Expense> expenses = s.filtered_expenses(req.has_param("category") ? req.get_param_value("category") : "");

        int month = 0;
        int year = 0;
        if (req.has_param("month")) {
            auto parsed = parse_month_year(req.get_param_value("month"));
            if (parsed.has_value()) {
                month = parsed->month;
                year = parsed->year;
            }
        }
        if (month == 0 || year == 0) {
            auto now = std::time(nullptr);
            std::tm local_tm{};
#ifdef _WIN32
            localtime_s(&local_tm, &now);
#else
            local_tm = *std::localtime(&now);
#endif
            month = local_tm.tm_mon + 1;
            year = local_tm.tm_year + 1900;
        }

        auto report = s.month_report(month, year);
        res.set_content(dashboard_page(req, s, expenses, report), "text/html");
    });

    app.Post("/expenses/add", [&](const httplib::Request& req, httplib::Response& res) {
        auto username = get_user_from_session(req);
        if (!username.has_value()) {
            res.set_redirect("/login");
            return;
        }

        UserSession s(username.value(), storage);
        s.load();

        try {
            const std::string date_str = req.get_param_value("date");
            const std::string category = req.get_param_value("category");
            const std::string amount_str = req.get_param_value("amount");
            const std::string note = req.has_param("note") ? req.get_param_value("note") : "";

            const int date_key = parse_date_to_key(date_str);
            const double amount = std::stod(amount_str);
            if (category.empty()) {
                throw std::runtime_error("Category is required");
            }
            if (amount <= 0.0) {
                throw std::runtime_error("Amount must be greater than 0");
            }

            s.add_expense(Expense{gen_id(), date_key, category, amount, note});
            s.save();

            res.set_redirect(make_redirect_with_msg("/dashboard", "success", "Expense added.").c_str());
        } catch (const std::exception& ex) {
            res.set_redirect(make_redirect_with_msg("/dashboard", "error", std::string("Could not add expense: ") + ex.what()).c_str());
        }
    });

    app.Post("/expenses/delete", [&](const httplib::Request& req, httplib::Response& res) {
        auto username = get_user_from_session(req);
        if (!username.has_value()) {
            res.set_redirect("/login");
            return;
        }

        UserSession s(username.value(), storage);
        s.load();

        const std::string expense_id = req.has_param("expense_id") ? req.get_param_value("expense_id") : "";
        if (expense_id.empty()) {
            res.set_redirect(make_redirect_with_msg("/dashboard", "warning", "Expense not found.").c_str());
            return;
        }

        if (s.delete_expense(expense_id)) {
            s.save();
            res.set_redirect(make_redirect_with_msg("/dashboard", "success", "Expense deleted.").c_str());
            return;
        }

        res.set_redirect(make_redirect_with_msg("/dashboard", "warning", "Expense not found.").c_str());
    });

    app.Post("/budget", [&](const httplib::Request& req, httplib::Response& res) {
        auto username = get_user_from_session(req);
        if (!username.has_value()) {
            res.set_redirect("/login");
            return;
        }

        UserSession s(username.value(), storage);
        s.load();

        try {
            const double budget = std::stod(req.get_param_value("budget"));
            if (budget <= 0.0) {
                throw std::runtime_error("Budget must be greater than 0");
            }

            s.set_budget(budget);
            s.save();
            res.set_redirect(make_redirect_with_msg("/dashboard", "success", "Budget updated.").c_str());
        } catch (const std::exception& ex) {
            res.set_redirect(make_redirect_with_msg("/dashboard", "error", std::string("Invalid budget: ") + ex.what()).c_str());
        }
    });

    app.Get("/export", [&](const httplib::Request& req, httplib::Response& res) {
        auto username = get_user_from_session(req);
        if (!username.has_value()) {
            res.set_redirect("/login");
            return;
        }

        UserSession s(username.value(), storage);
        s.load();

        std::ostringstream csv;
        csv << "id,date,category,amount,note\n";
        for (const auto& e : s.expenses()) {
            std::string note = e.note;
            for (char& c : note) {
                if (c == '"') {
                    c = '\'';
                }
            }
            csv << '"' << e.id << "\"," << '"' << key_to_date_string(e.date_key) << "\"," << '"'
                << e.category << "\"," << format_money(e.amount) << ",\"" << note << "\"\n";
        }

        res.set_header("Content-Disposition", "attachment; filename=expense_report.csv");
        res.set_content(csv.str(), "text/csv");
    });

    constexpr int port = 8080;
    std::cout << "Expense Tracker (C++) running on http://127.0.0.1:" << port << '\n';
    std::cout << "Press Ctrl+C to stop.\n";
    app.listen("127.0.0.1", port);

    return 0;
}
