# Expense Tracker (C++ Only)

A C++ web app equivalent of the attached Python Expense Tracker project.

## Features

- User registration and login
- Add and delete expenses
- Monthly budget update
- Monthly report:
  - total spend
  - remaining / over budget
  - top 3 categories
  - highest spending day
- Filter expenses by category and month
- Export expenses to CSV
- JSON file persistence under `data/`

## Tech

- C++17
- cpp-httplib (HTTP server)
- nlohmann/json (JSON persistence)
- CMake

## Folder Layout

- `src/` C++ source files
- `static/styles.css` UI styles
- `data/users.json` credential registry
- `data/<username>.json` per-user budget and expenses

## Build

From `d:/Project/expense-tracker-cpp`:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

```powershell
./build/Release/expense_tracker_cpp.exe
```

If your generator is Ninja/Makefiles, executable may be in `build/expense_tracker_cpp.exe`.

Open:

- http://127.0.0.1:8080

## Notes

- Run from the project root so relative `data/` and `static/` paths resolve correctly.
- Password hashing in this demo uses a deterministic app-level hash function (for local development only).
