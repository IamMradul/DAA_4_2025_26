# Expense Tracker Web App

A clean, browser-based expense tracker built with Flask.

This project includes:
- A web interface for managing expenses
- User authentication (register/login)
- Monthly reporting with top categories
- Budget tracking
- CSV export
- Existing CLI version (still available)

## Features

- Register and login with username/password
- Add expenses with date, category, amount, and note
- Delete expenses
- Set and update monthly budget
- View monthly summary:
  - total spend
  - over-budget or remaining amount
  - top 3 categories
  - highest spending day
- Filter expense list by category and month
- Export all expenses to CSV

## Tech Stack

- Python 3
- Flask
- Jinja2 templates
- CSS (custom, responsive UI)

## Project Structure

- `web_app.py` : Flask web server and routes
- `templates/` : HTML templates
- `static/styles.css` : UI styles
- `main.py` : CLI app and shared `Session` logic
- `models.py` : Expense model and AVL tree
- `storage.py` : file-based persistence
- `auth.py` : registration and login helpers
- `utils.py` : date/id/hash utilities
- `data/` : user and expense data files

## Setup

### 1. Open project folder

```powershell
cd d:\Project\expense-tracker
```

### 2. Create virtual environment (recommended)

```powershell
python -m venv .venv
```

### 3. Activate environment

PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

Git Bash / MSYS (if you use it):

```bash
source .venv/bin/activate
```

### 4. Install dependencies

```powershell
python -m pip install -r requirements.txt
```

## Run the Website

```powershell
python web_app.py
```

Then open:

```text
http://127.0.0.1:5000
```

## How to Use

1. Register a new account.
2. Login.
3. Add expenses from the dashboard form.
4. Set your monthly budget.
5. Use filters for category/month when needed.
6. Export CSV from the dashboard.

## Data Storage

Data is stored locally under `data/`:
- `users.json` stores user credentials (hashed password)
- `<username>.json` stores budget and expense list

No external database is required.

## Security Notes

- This is a local development app.
- Update `SECRET_KEY` in `web_app.py` before any production use.
- Use HTTPS and a production WSGI server if deploying publicly.

## Run the CLI Version (Optional)

```powershell
python main.py
```

## Troubleshooting

### `ModuleNotFoundError: No module named 'flask'`

Install dependencies:

```powershell
python -m pip install -r requirements.txt
```

### Port 5000 already in use

Run on another port:

```powershell
python -c "from web_app import app; app.run(debug=True, port=5001)"
```

Then open `http://127.0.0.1:5001`.

## Future Improvements

- Edit expense entries
- Category analytics charts
- Stronger auth/session hardening
- Production deployment config
