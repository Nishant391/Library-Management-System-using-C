============================================================
         LIBRARY MANAGEMENT SYSTEM — README
         Language : C  |  File Handling : Binary
============================================================

PROJECT FILES
-------------
  main.c                  — Complete source code (all modules)
  seed_books.c            — Terminal CLI utility to batch add/seed books
  seed_students_issues.c  — Terminal CLI utility to register students & issue books
  books.dat               — Binary file: Book records
  students.dat            — Binary file: Student records
  issues.dat              — Binary file: Issue/Return records
  LibraryMS.exe           — Compiled main application executable
  seed_books.exe          — Compiled book adding CLI utility

HOW TO COMPILE
--------------
  Using GCC (MinGW / MSYS2 on Windows):
    gcc -Wall -std=c11 -o LibraryMS main.c

  Using Code::Blocks:
    1. Create a new "Empty Project" (C language).
    2. Add main.c to the project.
    3. Press F9 to Build & Run.

  Using VS Code:
    1. Install the C/C++ extension.
    2. Install MinGW (GCC for Windows).
    3. Open the folder, open main.c.
    4. Press Ctrl+Shift+B and run the default build task,
       or open the terminal and type:
         gcc -Wall -std=c11 -o LibraryMS main.c
         ./LibraryMS

HOW TO RUN
----------
  Windows: double-click LibraryMS.exe,
           or run  ./LibraryMS  in the terminal.

DATA FILES
----------
  All data is stored in binary files in the same folder.
  Do NOT edit them with a text editor.

FINE POLICY
-----------
  Standard borrowing period : 14 days
  Fine rate                 : Rs. 5 per overdue day
  Fine is calculated automatically from system date.

============================================================

