from __future__ import annotations
from dataclasses import dataclass, asdict
from typing import Optional, List, Dict, Any
from utils import date_to_key, key_to_date
from datetime import date

@dataclass
class Expense:
    id: str
    date_key: int   
    category: str
    amount: float
    note: str

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "date_key": self.date_key,
            "category": self.category,
            "amount": self.amount,
            "note": self.note
        }

    @staticmethod
    def from_dict(d: Dict[str, Any]) -> "Expense":
        return Expense(
            id=d["id"],
            date_key=d["date_key"],
            category=d["category"],
            amount=float(d["amount"]),
            note=d.get("note", "")
        )

class AVLNode:
    def __init__(self, key: int):
        self.key = key
        self.expenses: List[Expense] = []  
        self.left: Optional[AVLNode] = None
        self.right: Optional[AVLNode] = None
        self.height = 1

    def to_dict(self):
        return {
            "key": self.key,
            "expenses": [e.to_dict() for e in self.expenses],
            "left": self.left.to_dict() if self.left else None,
            "right": self.right.to_dict() if self.right else None,
            "height": self.height
        }

class AVLTree:
    def __init__(self):
        self.root: Optional[AVLNode] = None

    def _height(self, node: Optional[AVLNode]) -> int:
        return node.height if node else 0

    def _update_height(self, node: AVLNode) -> None:
        node.height = 1 + max(self._height(node.left), self._height(node.right))

    def _balance_factor(self, node: AVLNode) -> int:
        return self._height(node.left) - self._height(node.right)

    def _rotate_right(self, y: AVLNode) -> AVLNode:
        x = y.left
        T2 = x.right
        x.right = y
        y.left = T2
        self._update_height(y)
        self._update_height(x)
        return x

    def _rotate_left(self, x: AVLNode) -> AVLNode:
        y = x.right
        T2 = y.left
        y.left = x
        x.right = T2
        self._update_height(x)
        self._update_height(y)
        return y

    def _balance(self, node: AVLNode) -> AVLNode:
        self._update_height(node)
        bf = self._balance_factor(node)
        if bf > 1:
            if self._balance_factor(node.left) < 0:
                node.left = self._rotate_left(node.left)
            return self._rotate_right(node)
        if bf < -1:
            if self._balance_factor(node.right) > 0:
                node.right = self._rotate_right(node.right)
            return self._rotate_left(node)
        return node

    def insert_expense(self, expense: Expense) -> None:
        self.root = self._insert(self.root, expense)

    def _insert(self, node: Optional[AVLNode], expense: Expense) -> AVLNode:
        if not node:
            node = AVLNode(expense.date_key)
            node.expenses.append(expense)
            return node
        if expense.date_key < node.key:
            node.left = self._insert(node.left, expense)
        elif expense.date_key > node.key:
            node.right = self._insert(node.right, expense)
        else:
            node.expenses.append(expense)
            return node
        return self._balance(node)

    def delete_expense(self, expense_id: str, date_key: int) -> bool:
        self.root, deleted = self._delete(self.root, expense_id, date_key)
        return deleted

    def _delete(self, node: Optional[AVLNode], expense_id: str, date_key: int):
        if not node:
            return node, False
        deleted = False
        if date_key < node.key:
            node.left, deleted = self._delete(node.left, expense_id, date_key)
        elif date_key > node.key:
            node.right, deleted = self._delete(node.right, expense_id, date_key)
        else:
            before = len(node.expenses)
            node.expenses = [e for e in node.expenses if e.id != expense_id]
            after = len(node.expenses)
            deleted = (after < before)
            if not node.expenses:
                if not node.left:
                    return node.right, True
                elif not node.right:
                    return node.left, True
                else:
                    succ = self._min_value_node(node.right)
                    node.key = succ.key
                    node.expenses = succ.expenses
                    node.right, _ = self._delete_node_by_key(node.right, succ.key)
        if node:
            node = self._balance(node)
        return node, deleted

    def _delete_node_by_key(self, node: Optional[AVLNode], key: int):
        if not node:
            return node, False
        deleted = False
        if key < node.key:
            node.left, deleted = self._delete_node_by_key(node.left, key)
        elif key > node.key:
            node.right, deleted = self._delete_node_by_key(node.right, key)
        else:
            deleted = True
            if not node.left:
                return node.right, True
            elif not node.right:
                return node.left, True
            else:
                succ = self._min_value_node(node.right)
                node.key = succ.key
                node.expenses = succ.expenses
                node.right, _ = self._delete_node_by_key(node.right, succ.key)
        if node:
            node = self._balance(node)
        return node, deleted

    def _min_value_node(self, node: AVLNode) -> AVLNode:
        current = node
        while current.left:
            current = current.left
        return current

    def inorder(self) -> List[Expense]:
        res = []
        self._inorder(self.root, res)
        return res

    def _inorder(self, node: Optional[AVLNode], res: List[Expense]) -> None:
        if not node:
            return
        self._inorder(node.left, res)
        res.extend(node.expenses)
        self._inorder(node.right, res)
    def range_query(self, low_key: int, high_key: int) -> List[Expense]:
        res = []
        self._range_query(self.root, low_key, high_key, res)
        return res

    def _range_query(self, node: Optional[AVLNode], low: int, high: int, res: List[Expense]) -> None:
        if not node:
            return
        if low < node.key:
            self._range_query(node.left, low, high, res)
        if low <= node.key <= high:
            res.extend(node.expenses)
        if node.key < high:
            self._range_query(node.right, low, high, res)

    def to_list(self) -> List[dict]:
        return [e.to_dict() for e in self.inorder()]

    def load_from_list(self, lst: List[dict]) -> None:
        self.root = None
        for d in lst:
            exp = Expense.from_dict(d)
            self.insert_expense(exp)
