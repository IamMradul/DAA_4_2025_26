import os
import json
from typing import Optional, Dict, Any
from models import Expense, AVLTree
from utils import gen_id
from hashlib import sha256

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, "data")
USERS_FILE = os.path.join(DATA_DIR, "users.json")

def ensure_data_dir():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)
    if not os.path.exists(USERS_FILE):
        with open(USERS_FILE, "w") as f:
            json.dump({}, f)

ensure_data_dir()

def load_users_registry() -> Dict[str, str]:
    with open(USERS_FILE, "r") as f:
        return json.load(f)

def save_users_registry(reg: Dict[str, str]) -> None:
    with open(USERS_FILE, "w") as f:
        json.dump(reg, f, indent=2)

def user_exists(username: str) -> bool:
    reg = load_users_registry()
    return username in reg

def get_user_hashed_password(username: str) -> Optional[str]:
    reg = load_users_registry()
    return reg.get(username)

def register_user(username: str, hashed_password: str) -> None:
    reg = load_users_registry()
    reg[username] = hashed_password
    save_users_registry(reg)
    user_file = os.path.join(DATA_DIR, f"{username}.json")
    if not os.path.exists(user_file):
        with open(user_file, "w") as f:
            initial = {
                "budget": None,
                "expenses": []
            }
            json.dump(initial, f, indent=2)

def load_user_data(username: str) -> Dict[str, Any]:
    user_file = os.path.join(DATA_DIR, f"{username}.json")
    if not os.path.exists(user_file):
        return {"budget": None, "expenses": []}
    with open(user_file, "r") as f:
        return json.load(f)

def save_user_data(username: str, budget, expense_list) -> None:
    user_file = os.path.join(DATA_DIR, f"{username}.json")
    payload = {
        "budget": budget,
        "expenses": expense_list 
    }
    with open(user_file, "w") as f:
        json.dump(payload, f, indent=2)
