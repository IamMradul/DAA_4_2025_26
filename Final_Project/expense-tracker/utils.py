from datetime import datetime, date
import hashlib
import uuid
from typing import Tuple

DATE_FORMAT = "%d-%m-%Y"  

def parse_date(s: str) -> date:
    return datetime.strptime(s, DATE_FORMAT).date()

from datetime import datetime

def date_to_key(date_input):
    """Convert a date, datetime, or DD-MM-YYYY string into YYYYMMDD integer key."""
    from datetime import datetime, date

    if isinstance(date_input, (datetime, date)):
        return int(date_input.strftime("%Y%m%d"))
    elif isinstance(date_input, str):
        dt = datetime.strptime(date_input, "%d-%m-%Y")
        return int(dt.strftime("%Y%m%d"))
    else:
        raise TypeError("date_to_key() expects str, date, or datetime")



def key_to_date(key: int) -> date:
    y = key // 10000
    m = (key % 10000) // 100
    d = key % 100
    return date(y, m, d)

def hash_password(plain: str) -> str:
    return hashlib.sha256(plain.encode('utf-8')).hexdigest()

def gen_id() -> str:
    return uuid.uuid4().hex
