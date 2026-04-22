from utils import hash_password
import storage

def register(username: str, password: str) -> bool:
    if storage.user_exists(username):
        return False
    hp = hash_password(password)
    storage.register_user(username, hp)
    return True

def login(username: str, password: str) -> bool:
    hp = storage.get_user_hashed_password(username)
    if not hp:
        return False
    return hp == hash_password(password)
