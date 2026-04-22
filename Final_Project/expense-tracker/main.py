# main.py
from utils import parse_date, date_to_key, gen_id, key_to_date
from models import Expense, AVLTree
import storage, auth
from collections import defaultdict
import heapq
from typing import Dict, List
import sys
import os

class Session:
    def __init__(self, username: str):
        self.username = username
        self.budget = None
        self.tree = AVLTree()
        self.category_map: Dict[str, List[Expense]] = defaultdict(list)
        self.undo_stack = []  
    def load(self):
        data = storage.load_user_data(self.username)
        self.budget = data.get("budget")
        exp_list = data.get("expenses", [])
        self.tree.load_from_list(exp_list)
        for ed in exp_list:
            e = Expense.from_dict(ed)
            self.category_map[e.category].append(e)

    def save(self):
        exp_list = self.tree.to_list()
        storage.save_user_data(self.username, self.budget, exp_list)

class UI:
    WIDTH = 76
    RESET = "\033[0m"
    BOLD = "\033[1m"
    DIM = "\033[2m"
    CYAN = "\033[36m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    RED = "\033[31m"

    @staticmethod
    def clear():
        os.system("cls" if os.name == "nt" else "clear")

    @staticmethod
    def line(ch: str = "-"):
        print(ch * UI.WIDTH)

    @staticmethod
    def header(title: str):
        UI.clear()
        UI.line("=")
        print(f"{UI.BOLD}{UI.CYAN}  {title}{UI.RESET}")
        UI.line("=")

    @staticmethod
    def section(title: str):
        print(f"\n{UI.BOLD}{title}{UI.RESET}")
        UI.line("-")

    @staticmethod
    def ok(msg: str):
        print(f"{UI.GREEN}{msg}{UI.RESET}")

    @staticmethod
    def warn(msg: str):
        print(f"{UI.YELLOW}{msg}{UI.RESET}")

    @staticmethod
    def err(msg: str):
        print(f"{UI.RED}{msg}{UI.RESET}")

    @staticmethod
    def pause():
        input(f"\n{UI.DIM}Press Enter to continue...{UI.RESET}")

    @staticmethod
    def prompt(label: str) -> str:
        return input(f"{UI.DIM}{label}{UI.RESET}").strip()

    @staticmethod
    def print_expenses_table(expenses: List[Expense], include_category: bool = True):
        if not expenses:
            UI.warn("No expenses recorded.")
            return

        if include_category:
            print(f"{'ID':<12} {'Date':<12} {'Category':<16} {'Amount':<12} Note")
            UI.line()
            for e in expenses:
                print(
                    f"{(e.id[:8] + '...'):<12} "
                    f"{key_to_date(e.date_key).strftime('%d-%m-%Y'):<12} "
                    f"{e.category[:15]:<16} "
                    f"{'₹' + format(e.amount, '.2f'):<12} "
                    f"{e.note}"
                )
        else:
            print(f"{'ID':<12} {'Date':<12} {'Amount':<12} Note")
            UI.line()
            for e in expenses:
                print(
                    f"{(e.id[:8] + '...'):<12} "
                    f"{key_to_date(e.date_key).strftime('%d-%m-%Y'):<12} "
                    f"{'₹' + format(e.amount, '.2f'):<12} "
                    f"{e.note}"
                )

def register_flow():
    UI.section("Register")
    username = UI.prompt("Choose username: ")
    password = UI.prompt("Choose password: ")
    if auth.register(username, password):
        UI.ok("Registered successfully. Please login.")
    else:
        UI.warn("Username already exists.")

def login_flow():
    UI.section("Login")
    username = UI.prompt("Username: ")
    password = UI.prompt("Password: ")
    if auth.login(username, password):
        UI.ok("Login successful.")
        return username
    UI.err("Invalid credentials.")
    return None

def add_expense_flow(session: Session):
    try:
        UI.section("Add Expense")
        dstr = UI.prompt("Date (DD-MM-YYYY): ")
        dt = parse_date(dstr)
        key = date_to_key(dt)
        cat = UI.prompt("Category: ")
        amt = float(UI.prompt("Amount: "))
        note = UI.prompt("Note (optional): ")
        eid = gen_id()
        exp = Expense(id=eid, date_key=key, category=cat, amount=amt, note=note)
        session.tree.insert_expense(exp)
        session.category_map[cat].append(exp)
        session.undo_stack.append(("add", exp.to_dict()))
        session.save()
        UI.ok("Expense added.")
        if session.budget is not None:
            month_start = key - (key % 100) + 1
            y = dt.year
            m = dt.month
            
            import calendar
            first_day = date_to_key(dt.replace(day=1))
            last_day = date_to_key(dt.replace(day=calendar.monthrange(y, m)[1]))
            month_total = sum(e.amount for e in session.tree.range_query(first_day, last_day))
            remaining = session.budget - month_total
            if remaining < 0:
                UI.warn(f"Alert: you've exceeded the monthly budget by ₹{-remaining:.2f}")
            else:
                UI.ok(f"Remaining budget this month: ₹{remaining:.2f}")
    except Exception as ex:
        UI.err(f"Error adding expense: {ex}")

def delete_expense_flow(session: Session):
    try:
        UI.section("Delete Expense")
        eid = UI.prompt("Enter Expense ID to delete: ")
        found = None
        for e in session.tree.inorder():
            if e.id == eid:
                found = e
                break
        if not found:
            UI.warn("Expense not found.")
            return
        ok = session.tree.delete_expense(eid, found.date_key)
        if ok:
            session.category_map[found.category] = [x for x in session.category_map[found.category] if x.id != eid]
            session.undo_stack.append(("delete", found.to_dict()))
            session.save()
            UI.ok("Deleted successfully.")
        else:
            UI.warn("Could not delete.")
    except Exception as ex:
        UI.err(f"Error deleting expense: {ex}")

def undo_flow(session: Session):
    UI.section("Undo Last Action")
    if not session.undo_stack:
        UI.warn("Nothing to undo.")
        return
    action, payload = session.undo_stack.pop()
    if action == "add":
        eid = payload["id"]
        key = payload["date_key"]
        deleted = session.tree.delete_expense(eid, key)
        if deleted:
            session.category_map[payload["category"]] = [x for x in session.category_map[payload["category"]] if x.id != eid]
            session.save()
            UI.ok("Undo: added expense removed.")
        else:
            UI.err("Undo failed.")
    elif action == "delete":
        e = Expense.from_dict(payload)
        session.tree.insert_expense(e)
        session.category_map[e.category].append(e)
        session.save()
        UI.ok("Undo: deleted expense restored.")

def view_month_report(session: Session):
    import calendar
    from datetime import date
    UI.section("Monthly Report")
    choice = UI.prompt("Enter month and year (MM-YYYY) or press enter for current month: ")
    if choice:
        mm, yy = choice.split("-")
        m = int(mm); y = int(yy)
    else:
        today = date.today()
        m = today.month; y = today.year
    start_key = date_to_key(date(y, m, 1))
    last_day = calendar.monthrange(y, m)[1]
    end_key = date_to_key(date(y, m, last_day))
    expenses = session.tree.range_query(start_key, end_key)
    total = sum(e.amount for e in expenses)
    print(f"Report for {m:02d}-{y}")
    UI.line()
    print(f"Total spent: ₹{total:.2f}")
    if session.budget is not None:
        print(f"Budget: ₹{session.budget:.2f}")
        rem = session.budget - total
        if rem < 0:
            UI.warn(f"Over budget by: ₹{-rem:.2f}")
        else:
            UI.ok(f"Remaining: ₹{rem:.2f}")
    cat_sums = {}
    for e in expenses:
        cat_sums[e.category] = cat_sums.get(e.category, 0.0) + e.amount
    if cat_sums:
        top = heapq.nlargest(3, cat_sums.items(), key=lambda x: x[1])
        print("Top categories:")
        for i, (c, amt) in enumerate(top, 1):
            print(f"{i}. {c} - ₹{amt:.2f}")
    day_sums = {}
    for e in expenses:
        day_sums[e.date_key] = day_sums.get(e.date_key, 0.0) + e.amount
    if day_sums:
        best_key, best_amt = max(day_sums.items(), key=lambda x: x[1])
        print(f"Highest spend day: {key_to_date(best_key).strftime('%d-%m-%Y')} (₹{best_amt:.2f})")

def list_all_expenses(session: Session):
    UI.section("All Expenses")
    exps = session.tree.inorder()
    UI.print_expenses_table(exps, include_category=True)

def set_budget_flow(session: Session):
    try:
        UI.section("Set or Update Budget")
        b = float(UI.prompt("Enter monthly budget amount (₹): "))
        session.budget = b
        session.save()
        UI.ok("Budget set.")
    except Exception as ex:
        UI.err(f"Invalid input: {ex}")

def search_by_category(session: Session):
    UI.section("Search by Category")
    cat = UI.prompt("Enter category to search: ")
    lst = session.category_map.get(cat, [])
    if not lst:
        UI.warn("No expenses in this category.")
        return
    UI.print_expenses_table(lst, include_category=False)

def export_csv(session: Session):
    import csv
    UI.section("Export CSV")
    fname = UI.prompt("Enter filename (e.g., report.csv): ")
    exps = session.tree.inorder()
    with open(fname, "w", newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(["id", "date", "category", "amount", "note"])
        for e in exps:
            writer.writerow([e.id, key_to_date(e.date_key).strftime("%d-%m-%Y"), e.category, f"{e.amount:.2f}", e.note])
    UI.ok(f"Exported to {fname}")

def main_menu():
    while True:
        UI.header("Expense Tracker")
        print("1. Register")
        print("2. Login")
        print("3. Exit")
        UI.line()
        ch = UI.prompt("Choice: ")
        if ch == "1":
            register_flow()
            UI.pause()
        elif ch == "2":
            user = login_flow()
            if user:
                session = Session(user)
                session.load()
                user_menu(session)
            else:
                UI.pause()
        elif ch == "3":
            UI.ok("Bye.")
            sys.exit(0)
        else:
            UI.warn("Invalid choice.")
            UI.pause()

def user_menu(session: Session):
    while True:
        UI.header(f"Welcome, {session.username}")
        print("1. Add Expense")
        print("2. Delete Expense (by ID)")
        print("3. View Monthly Report")
        print("4. Set/Update Budget")
        print("5. List All Expenses")
        print("6. Search by Category")
        print("7. Undo Last Action")
        print("8. Export CSV")
        print("9. Logout")
        UI.line()
        ch = UI.prompt("Choice: ")
        if ch == "1":
            add_expense_flow(session)
            UI.pause()
        elif ch == "2":
            delete_expense_flow(session)
            UI.pause()
        elif ch == "3":
            view_month_report(session)
            UI.pause()
        elif ch == "4":
            set_budget_flow(session)
            UI.pause()
        elif ch == "5":
            list_all_expenses(session)
            UI.pause()
        elif ch == "6":
            search_by_category(session)
            UI.pause()
        elif ch == "7":
            undo_flow(session)
            UI.pause()
        elif ch == "8":
            export_csv(session)
            UI.pause()
        elif ch == "9":
            UI.ok("Logging out.")
            UI.pause()
            break
        else:
            UI.warn("Invalid choice.")
            UI.pause()

if __name__ == "__main__":
    main_menu()
