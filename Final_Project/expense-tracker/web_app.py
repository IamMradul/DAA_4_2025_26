from datetime import date
from io import StringIO
import csv

from flask import Flask, flash, redirect, render_template, request, session as flask_session, url_for, Response

import auth
from main import Session
from models import Expense
from utils import parse_date, date_to_key, key_to_date, gen_id

app = Flask(__name__)
app.config["SECRET_KEY"] = "expense-tracker-secret-key-change-me"


def current_user() -> str | None:
    return flask_session.get("username")


def require_login():
    if not current_user():
        return redirect(url_for("login"))
    return None


def load_session(username: str) -> Session:
    s = Session(username)
    s.load()
    return s


def month_report_data(s: Session, month: int, year: int):
    import calendar

    start_key = date_to_key(date(year, month, 1))
    end_key = date_to_key(date(year, month, calendar.monthrange(year, month)[1]))
    expenses = s.tree.range_query(start_key, end_key)
    total = sum(e.amount for e in expenses)

    cat_sums = {}
    for e in expenses:
        cat_sums[e.category] = cat_sums.get(e.category, 0.0) + e.amount
    top_categories = sorted(cat_sums.items(), key=lambda x: x[1], reverse=True)[:3]

    day_sums = {}
    for e in expenses:
        day_sums[e.date_key] = day_sums.get(e.date_key, 0.0) + e.amount

    highest_day = None
    if day_sums:
        best_key, best_amount = max(day_sums.items(), key=lambda x: x[1])
        highest_day = {
            "date": key_to_date(best_key).strftime("%d-%m-%Y"),
            "amount": best_amount,
        }

    return {
        "month": month,
        "year": year,
        "total": total,
        "top_categories": top_categories,
        "highest_day": highest_day,
        "expenses": expenses,
    }


@app.route("/")
def home():
    if current_user():
        return redirect(url_for("dashboard"))
    return redirect(url_for("login"))


@app.route("/register", methods=["GET", "POST"])
def register():
    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()

        if not username or not password:
            flash("Username and password are required.", "error")
            return render_template("register.html")

        if auth.register(username, password):
            flash("Registration successful. Please login.", "success")
            return redirect(url_for("login"))

        flash("Username already exists.", "warning")

    return render_template("register.html")


@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        username = request.form.get("username", "").strip()
        password = request.form.get("password", "").strip()

        if auth.login(username, password):
            flask_session["username"] = username
            flash("Login successful.", "success")
            return redirect(url_for("dashboard"))

        flash("Invalid credentials.", "error")

    return render_template("login.html")


@app.route("/logout")
def logout():
    flask_session.pop("username", None)
    flash("Logged out.", "success")
    return redirect(url_for("login"))


@app.route("/dashboard")
def dashboard():
    gate = require_login()
    if gate:
        return gate

    username = current_user()
    s = load_session(username)

    query_category = request.args.get("category", "").strip()
    filter_month = request.args.get("month", "").strip()

    expenses = s.tree.inorder()
    if query_category:
        expenses = [e for e in expenses if e.category.lower() == query_category.lower()]

    if filter_month:
        try:
            mm, yy = filter_month.split("-")
            m = int(mm)
            y = int(yy)
            report = month_report_data(s, m, y)
        except Exception:
            flash("Invalid month format. Use MM-YYYY.", "warning")
            today = date.today()
            report = month_report_data(s, today.month, today.year)
    else:
        today = date.today()
        report = month_report_data(s, today.month, today.year)

    return render_template(
        "dashboard.html",
        username=username,
        expenses=expenses,
        budget=s.budget,
        report=report,
        selected_category=query_category,
        selected_month=filter_month,
    )


@app.route("/expenses/add", methods=["POST"])
def add_expense():
    gate = require_login()
    if gate:
        return gate

    username = current_user()
    s = load_session(username)

    try:
        dt = parse_date(request.form.get("date", "").strip())
        category = request.form.get("category", "").strip()
        amount = float(request.form.get("amount", "").strip())
        note = request.form.get("note", "").strip()

        if not category:
            raise ValueError("Category is required")
        if amount <= 0:
            raise ValueError("Amount must be greater than 0")

        exp = Expense(
            id=gen_id(),
            date_key=date_to_key(dt),
            category=category,
            amount=amount,
            note=note,
        )
        s.tree.insert_expense(exp)
        s.category_map[category].append(exp)
        s.save()
        flash("Expense added.", "success")
    except Exception as ex:
        flash(f"Could not add expense: {ex}", "error")

    return redirect(url_for("dashboard"))


@app.route("/expenses/delete/<expense_id>", methods=["POST"])
def delete_expense(expense_id: str):
    gate = require_login()
    if gate:
        return gate

    username = current_user()
    s = load_session(username)

    found = None
    for e in s.tree.inorder():
        if e.id == expense_id:
            found = e
            break

    if not found:
        flash("Expense not found.", "warning")
        return redirect(url_for("dashboard"))

    if s.tree.delete_expense(expense_id, found.date_key):
        s.save()
        flash("Expense deleted.", "success")
    else:
        flash("Could not delete expense.", "error")

    return redirect(url_for("dashboard"))


@app.route("/budget", methods=["POST"])
def set_budget():
    gate = require_login()
    if gate:
        return gate

    username = current_user()
    s = load_session(username)

    try:
        amount = float(request.form.get("budget", "").strip())
        if amount <= 0:
            raise ValueError("Budget must be greater than 0")
        s.budget = amount
        s.save()
        flash("Budget updated.", "success")
    except Exception as ex:
        flash(f"Invalid budget: {ex}", "error")

    return redirect(url_for("dashboard"))


@app.route("/export")
def export_csv():
    gate = require_login()
    if gate:
        return gate

    username = current_user()
    s = load_session(username)
    exps = s.tree.inorder()

    output = StringIO()
    writer = csv.writer(output)
    writer.writerow(["id", "date", "category", "amount", "note"])

    for e in exps:
        writer.writerow(
            [
                e.id,
                key_to_date(e.date_key).strftime("%d-%m-%Y"),
                e.category,
                f"{e.amount:.2f}",
                e.note,
            ]
        )

    output.seek(0)
    return Response(
        output.getvalue(),
        mimetype="text/csv",
        headers={"Content-Disposition": "attachment; filename=expense_report.csv"},
    )


if __name__ == "__main__":
    app.run(debug=True)
