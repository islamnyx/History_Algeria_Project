# Algeria History Database — GTK GUI
### NSCS · Algorithms & Dynamic Data Structures · 2025–2026

---

## How to Compile & Run

### 1. Install dependencies (Ubuntu/Debian)
```bash
sudo apt-get install -y libgtk-3-dev gcc make
```

### 2. Compile
```bash
make
```
Or manually:
```bash
gcc main_gui.c -o algeria_history $(pkg-config --cflags --libs gtk+-3.0) -lm
```

### 3. Run
```bash
./algeria_history
```
Or with a custom database file:
```bash
./algeria_history my_database.txt
```

---

## Project Structure

```
algeria_gui/
├── main_gui.c     ← Full GTK GUI application (all modules)
├── Makefile       ← Build system
├── database.txt   ← History data file
└── README.md      ← This file
```

---

## Modules Implemented

| Module | Functions |
|--------|-----------|
| Linked Lists | List, Search, Add, Delete, Sort alpha, Sort by age, Add Events |
| Stack | Build, Display, Search, Sort, Reverse (recursive), Delete, Overlapping dates, Shortest definition |
| Binary Search Tree | Build, In/Pre/Post-order, Search, Delete, Height & Size |
| Recursion | Count occurrences, Palindrome check, Permutations, Distinct subsequences, Remove occurrences |

---

## Database Format

```
Name=Definition=YearOfBirth=YearOfDeath        ← Personalities
EventName:Description{Year}                     ← Events
```

---

## Design
- **Theme**: Algerian desert gold + Ottoman manuscript dark aesthetic
- **Colors**: Deep mahogany backgrounds, gold borders, amber text
- **Font**: Monospace terminal output, clean sans-serif UI
- **Navigation**: Sidebar with module pages + shared output terminal

