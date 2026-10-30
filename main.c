/*
 * ============================================================
 *          LIBRARY MANAGEMENT SYSTEM
 *          Language : C (Standard C99/C11)
 *          File     : main.c
 *          Author   : College Mini Project
 *          Date     : 2026
 * ============================================================
 *
 * Features:
 *   - Book Management   (Add / Display / Search / Update / Delete)
 *   - Student Management (Register / Display / Search)
 *   - Issue & Return Books with automatic date calculation
 *   - Fine Calculation  (Rs.5 per day after 14-day grace period)
 *   - Author Directory
 *   - Binary File Handling (books.dat / students.dat / issues.dat)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* ============================================================
 *  CONSTANTS
 * ============================================================ */
#define MAX_TITLE      100
#define MAX_AUTHOR      80
#define MAX_PUBLISHER   80
#define MAX_NAME        80
#define MAX_DEPT        50
#define MAX_CONTACT     15
#define BORROW_DAYS     14        /* Standard borrowing period  */
#define FINE_PER_DAY     5        /* Fine in rupees per late day */

#define BOOKS_FILE    "books.dat"
#define STUDENTS_FILE "students.dat"
#define ISSUES_FILE   "issues.dat"

/* ============================================================
 *  STRUCTURES
 * ============================================================ */

/* Date structure for storing issue / due / return dates */
typedef struct {
    int day;
    int month;
    int year;
} Date;

/* Book record */
typedef struct {
    int    bookID;
    char   title[MAX_TITLE];
    char   author[MAX_AUTHOR];
    char   publisher[MAX_PUBLISHER];
    int    quantity;
    int    available;
} Book;

/* Student record */
typedef struct {
    int    studentID;
    char   name[MAX_NAME];
    char   department[MAX_DEPT];
    char   contact[MAX_CONTACT];
} Student;

/* Issue / Return record */
typedef struct {
    int    issueID;
    int    studentID;
    int    bookID;
    Date   issueDate;
    Date   dueDate;
    int    returned;          /* 0 = not returned, 1 = returned */
    Date   returnDate;        /* filled when the book is returned */
    long   fine;              /* fine amount in rupees            */
} Issue;

/* ============================================================
 *  UTILITY FUNCTION PROTOTYPES
 * ============================================================ */
void  clearScreen(void);
void  pauseScreen(void);
void  printLine(char ch, int len);
void  printHeader(const char *title);
void  flushInputBuffer(void);

Date  getCurrentDate(void);
long  daysBetween(Date d1, Date d2);
void  printDate(Date d);
int   isValidDate(Date d);

/* ============================================================
 *  BOOK MODULE PROTOTYPES
 * ============================================================ */
void  addBook(void);
void  displayAllBooks(void);
void  searchBook(void);
void  updateBook(void);
void  deleteBook(void);
int   bookExists(int id);
void  printBookHeader(void);
void  printBookRecord(Book *b);

/* ============================================================
 *  STUDENT MODULE PROTOTYPES
 * ============================================================ */
void  registerStudent(void);
void  displayAllStudents(void);
void  searchStudent(void);
int   studentExists(int id);
void  printStudentHeader(void);
void  printStudentRecord(Student *s);

/* ============================================================
 *  ISSUE / RETURN MODULE PROTOTYPES
 * ============================================================ */
void  issueBook(void);
void  returnBook(void);
int   getNextIssueID(void);
int   isBookIssuedToStudent(int studentID, int bookID, Issue *found);

/* ============================================================
 *  FINE MODULE PROTOTYPE
 * ============================================================ */
void  calculateFine(void);

/* ============================================================
 *  AUTHOR DIRECTORY PROTOTYPES
 * ============================================================ */
void  authorDirectory(void);
void  displayAllAuthors(void);
void  searchByAuthor(void);

/* ============================================================
 *  MAIN MENU
 * ============================================================ */
void  mainMenu(void);

/* ============================================================
 *  UTILITY FUNCTIONS — Implementation
 * ============================================================ */

void clearScreen(void) {
    system("cls");
}

void pauseScreen(void) {
    printf("\n  Press ENTER to continue...");
    flushInputBuffer();
    getchar();
}

void printLine(char ch, int len) {
    int i;
    for (i = 0; i < len; i++) putchar(ch);
    putchar('\n');
}

void printHeader(const char *title) {
    clearScreen();
    printLine('=', 60);
    printf("       LIBRARY MANAGEMENT SYSTEM\n");
    printLine('=', 60);
    printf("  >> %s\n", title);
    printLine('-', 60);
}

void flushInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Returns today's date using the system clock */
Date getCurrentDate(void) {
    Date d;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    d.day   = tm_info->tm_mday;
    d.month = tm_info->tm_mon + 1;
    d.year  = tm_info->tm_year + 1900;
    return d;
}

/* Converts a Date to "days since epoch" for arithmetic */
static long dateToEpochDays(Date d) {
    /* Zeller / proleptic Gregorian calendar formula */
    long y = d.year;
    long m = d.month;
    long day = d.day;
    if (m < 3) { y--; m += 12; }
    long a = y / 100;
    long b = 2 - a + a / 4;
    return (long)(365.25 * (y + 4716)) + (long)(30.6001 * (m + 1)) + day + b - 1524;
}

/* Returns d2 - d1 in days (positive means d2 is later) */
long daysBetween(Date d1, Date d2) {
    return dateToEpochDays(d2) - dateToEpochDays(d1);
}

void printDate(Date d) {
    printf("%02d/%02d/%04d", d.day, d.month, d.year);
}

int isValidDate(Date d) {
    if (d.year < 2000 || d.year > 2100) return 0;
    if (d.month < 1 || d.month > 12)   return 0;
    if (d.day   < 1 || d.day   > 31)   return 0;
    return 1;
}

/* ============================================================
 *  BOOK MODULE — Implementation
 * ============================================================ */

/* Returns 1 if a book with the given ID already exists */
int bookExists(int id) {
    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) return 0;
    Book b;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        if (b.bookID == id) { fclose(fp); return 1; }
    }
    fclose(fp);
    return 0;
}

void printBookHeader(void) {
    printf("\n  %-6s %-30s %-20s %-12s %5s %9s\n",
           "ID", "Title", "Author", "Publisher", "Qty", "Available");
    printLine('-', 90);
}

void printBookRecord(Book *b) {
    printf("  %-6d %-30s %-20s %-12s %5d %9d\n",
           b->bookID, b->title, b->author, b->publisher,
           b->quantity, b->available);
}

/* ADD NEW BOOK */
void addBook(void) {
    printHeader("ADD NEW BOOK");
    Book b;
    memset(&b, 0, sizeof(Book));

    printf("\n  Enter Book ID        : ");
    if (scanf("%d", &b.bookID) != 1) { flushInputBuffer(); printf("  [ERROR] Invalid ID.\n"); pauseScreen(); return; }
    flushInputBuffer();

    if (b.bookID <= 0) { printf("  [ERROR] Book ID must be a positive integer.\n"); pauseScreen(); return; }

    if (bookExists(b.bookID)) {
        printf("  [ERROR] Book with ID %d already exists.\n", b.bookID);
        pauseScreen();
        return;
    }

    printf("  Enter Book Title     : ");
    fgets(b.title, MAX_TITLE, stdin);
    b.title[strcspn(b.title, "\n")] = '\0';
    if (strlen(b.title) == 0) { printf("  [ERROR] Title cannot be empty.\n"); pauseScreen(); return; }

    printf("  Enter Author Name    : ");
    fgets(b.author, MAX_AUTHOR, stdin);
    b.author[strcspn(b.author, "\n")] = '\0';
    if (strlen(b.author) == 0) { printf("  [ERROR] Author cannot be empty.\n"); pauseScreen(); return; }

    printf("  Enter Publisher      : ");
    fgets(b.publisher, MAX_PUBLISHER, stdin);
    b.publisher[strcspn(b.publisher, "\n")] = '\0';
    if (strlen(b.publisher) == 0) { printf("  [ERROR] Publisher cannot be empty.\n"); pauseScreen(); return; }

    printf("  Enter Quantity       : ");
    if (scanf("%d", &b.quantity) != 1 || b.quantity <= 0) {
        flushInputBuffer();
        printf("  [ERROR] Quantity must be a positive integer.\n");
        pauseScreen(); return;
    }
    flushInputBuffer();

    b.available = b.quantity;

    FILE *fp = fopen(BOOKS_FILE, "ab");
    if (!fp) { printf("  [ERROR] Cannot open books file.\n"); pauseScreen(); return; }
    fwrite(&b, sizeof(Book), 1, fp);
    fclose(fp);

    printf("\n  [SUCCESS] Book '%s' added successfully!\n", b.title);
    pauseScreen();
}

/* DISPLAY ALL BOOKS */
void displayAllBooks(void) {
    printHeader("DISPLAY ALL BOOKS");
    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) { printf("\n  No books records found.\n"); pauseScreen(); return; }

    Book b;
    int count = 0;
    printBookHeader();
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        printBookRecord(&b);
        count++;
    }
    fclose(fp);

    if (count == 0) printf("  No books in the library.\n");
    else printf("\n  Total Books: %d\n", count);
    pauseScreen();
}

/* SEARCH BOOK */
void searchBook(void) {
    printHeader("SEARCH BOOK");
    printf("\n  Search By:\n");
    printf("    1. Book ID\n");
    printf("    2. Book Title\n");
    printf("\n  Enter choice: ");

    int choice;
    if (scanf("%d", &choice) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) { printf("\n  [ERROR] No book records found.\n"); pauseScreen(); return; }

    Book b;
    int found = 0;

    if (choice == 1) {
        int id;
        printf("  Enter Book ID: ");
        if (scanf("%d", &id) != 1) { flushInputBuffer(); fclose(fp); pauseScreen(); return; }
        flushInputBuffer();
        printBookHeader();
        while (fread(&b, sizeof(Book), 1, fp) == 1) {
            if (b.bookID == id) { printBookRecord(&b); found = 1; break; }
        }
    } else if (choice == 2) {
        char keyword[MAX_TITLE];
        printf("  Enter Book Title (or partial): ");
        fgets(keyword, MAX_TITLE, stdin);
        keyword[strcspn(keyword, "\n")] = '\0';
        /* Convert both to lowercase for case-insensitive compare */
        char kl[MAX_TITLE];
        strncpy(kl, keyword, MAX_TITLE);
        int i;
        for (i = 0; kl[i]; i++) kl[i] = (char)tolower((unsigned char)kl[i]);

        printBookHeader();
        while (fread(&b, sizeof(Book), 1, fp) == 1) {
            char tl[MAX_TITLE];
            strncpy(tl, b.title, MAX_TITLE);
            for (i = 0; tl[i]; i++) tl[i] = (char)tolower((unsigned char)tl[i]);
            if (strstr(tl, kl)) { printBookRecord(&b); found = 1; }
        }
    } else {
        printf("  [ERROR] Invalid choice.\n");
    }

    fclose(fp);
    if (!found && (choice == 1 || choice == 2))
        printf("  [INFO] No matching book found.\n");
    pauseScreen();
}

/* UPDATE BOOK */
void updateBook(void) {
    printHeader("UPDATE BOOK");
    int id;
    printf("\n  Enter Book ID to Update: ");
    if (scanf("%d", &id) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) { printf("  [ERROR] No book records found.\n"); pauseScreen(); return; }

    FILE *tmp = fopen("books_tmp.dat", "wb");
    if (!tmp) { fclose(fp); printf("  [ERROR] Cannot create temp file.\n"); pauseScreen(); return; }

    Book b;
    int found = 0;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        if (b.bookID == id) {
            found = 1;
            printf("\n  Current Details:\n");
            printBookHeader();
            printBookRecord(&b);
            printf("\n  Enter New Title     (current: %s): ", b.title);
            char tmp_str[MAX_TITLE];
            fgets(tmp_str, MAX_TITLE, stdin);
            tmp_str[strcspn(tmp_str, "\n")] = '\0';
            if (strlen(tmp_str) > 0) strncpy(b.title, tmp_str, MAX_TITLE);

            printf("  Enter New Author    (current: %s): ", b.author);
            fgets(tmp_str, MAX_AUTHOR, stdin);
            tmp_str[strcspn(tmp_str, "\n")] = '\0';
            if (strlen(tmp_str) > 0) strncpy(b.author, tmp_str, MAX_AUTHOR);

            printf("  Enter New Publisher (current: %s): ", b.publisher);
            fgets(tmp_str, MAX_PUBLISHER, stdin);
            tmp_str[strcspn(tmp_str, "\n")] = '\0';
            if (strlen(tmp_str) > 0) strncpy(b.publisher, tmp_str, MAX_PUBLISHER);

            printf("  Enter New Quantity  (current: %d, 0 = no change): ", b.quantity);
            int newQty;
            if (scanf("%d", &newQty) == 1 && newQty > 0) {
                /* Adjust available proportionally */
                int issued = b.quantity - b.available;
                b.quantity  = newQty;
                b.available = (newQty - issued) > 0 ? (newQty - issued) : 0;
            }
            flushInputBuffer();
        }
        fwrite(&b, sizeof(Book), 1, tmp);
    }

    fclose(fp);
    fclose(tmp);

    if (!found) {
        remove("books_tmp.dat");
        printf("  [ERROR] Book ID %d not found.\n", id);
    } else {
        remove(BOOKS_FILE);
        rename("books_tmp.dat", BOOKS_FILE);
        printf("\n  [SUCCESS] Book updated successfully.\n");
    }
    pauseScreen();
}

/* DELETE BOOK */
void deleteBook(void) {
    printHeader("DELETE BOOK");
    int id;
    printf("\n  Enter Book ID to Delete: ");
    if (scanf("%d", &id) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    if (!bookExists(id)) {
        printf("  [ERROR] Book ID %d not found.\n", id);
        pauseScreen(); return;
    }

    /* Safety: don't delete if copies are currently issued */
    FILE *ifp = fopen(ISSUES_FILE, "rb");
    if (ifp) {
        Issue iss;
        while (fread(&iss, sizeof(Issue), 1, ifp) == 1) {
            if (iss.bookID == id && !iss.returned) {
                fclose(ifp);
                printf("  [ERROR] Cannot delete — book has copies currently issued.\n");
                pauseScreen(); return;
            }
        }
        fclose(ifp);
    }

    char confirm;
    printf("  Are you sure you want to delete Book ID %d? (y/n): ", id);
    scanf(" %c", &confirm);
    flushInputBuffer();
    if (tolower((unsigned char)confirm) != 'y') { printf("  Deletion cancelled.\n"); pauseScreen(); return; }

    FILE *fp  = fopen(BOOKS_FILE, "rb");
    FILE *tmp = fopen("books_tmp.dat", "wb");
    if (!fp || !tmp) {
        if (fp)  fclose(fp);
        if (tmp) fclose(tmp);
        printf("  [ERROR] File operation failed.\n");
        pauseScreen(); return;
    }

    Book b;
    int deleted = 0;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        if (b.bookID == id) { deleted = 1; continue; }
        fwrite(&b, sizeof(Book), 1, tmp);
    }
    fclose(fp);
    fclose(tmp);
    remove(BOOKS_FILE);
    rename("books_tmp.dat", BOOKS_FILE);

    if (deleted) printf("\n  [SUCCESS] Book ID %d deleted successfully.\n", id);
    else         printf("  [ERROR] Book not found.\n");
    pauseScreen();
}

/* ============================================================
 *  STUDENT MODULE — Implementation
 * ============================================================ */

int studentExists(int id) {
    FILE *fp = fopen(STUDENTS_FILE, "rb");
    if (!fp) return 0;
    Student s;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.studentID == id) { fclose(fp); return 1; }
    }
    fclose(fp);
    return 0;
}

void printStudentHeader(void) {
    printf("\n  %-10s %-25s %-20s %-15s\n",
           "Stu.ID", "Name", "Department", "Contact");
    printLine('-', 75);
}

void printStudentRecord(Student *s) {
    printf("  %-10d %-25s %-20s %-15s\n",
           s->studentID, s->name, s->department, s->contact);
}

/* REGISTER STUDENT */
void registerStudent(void) {
    printHeader("REGISTER NEW STUDENT");
    Student s;
    memset(&s, 0, sizeof(Student));

    printf("\n  Enter Student ID     : ");
    if (scanf("%d", &s.studentID) != 1) { flushInputBuffer(); printf("  [ERROR] Invalid ID.\n"); pauseScreen(); return; }
    flushInputBuffer();

    if (s.studentID <= 0) { printf("  [ERROR] Student ID must be positive.\n"); pauseScreen(); return; }

    if (studentExists(s.studentID)) {
        printf("  [ERROR] Student with ID %d already registered.\n", s.studentID);
        pauseScreen(); return;
    }

    printf("  Enter Student Name   : ");
    fgets(s.name, MAX_NAME, stdin);
    s.name[strcspn(s.name, "\n")] = '\0';
    if (strlen(s.name) == 0) { printf("  [ERROR] Name cannot be empty.\n"); pauseScreen(); return; }

    printf("  Enter Department     : ");
    fgets(s.department, MAX_DEPT, stdin);
    s.department[strcspn(s.department, "\n")] = '\0';
    if (strlen(s.department) == 0) { printf("  [ERROR] Department cannot be empty.\n"); pauseScreen(); return; }

    printf("  Enter Contact Number : ");
    fgets(s.contact, MAX_CONTACT, stdin);
    s.contact[strcspn(s.contact, "\n")] = '\0';

    FILE *fp = fopen(STUDENTS_FILE, "ab");
    if (!fp) { printf("  [ERROR] Cannot open students file.\n"); pauseScreen(); return; }
    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);

    printf("\n  [SUCCESS] Student '%s' registered successfully!\n", s.name);
    pauseScreen();
}

/* DISPLAY ALL STUDENTS */
void displayAllStudents(void) {
    printHeader("ALL REGISTERED STUDENTS");
    FILE *fp = fopen(STUDENTS_FILE, "rb");
    if (!fp) { printf("\n  No student records found.\n"); pauseScreen(); return; }

    Student s;
    int count = 0;
    printStudentHeader();
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        printStudentRecord(&s);
        count++;
    }
    fclose(fp);
    if (count == 0) printf("  No students registered.\n");
    else printf("\n  Total Students: %d\n", count);
    pauseScreen();
}

/* SEARCH STUDENT */
void searchStudent(void) {
    printHeader("SEARCH STUDENT");
    int id;
    printf("\n  Enter Student ID: ");
    if (scanf("%d", &id) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    FILE *fp = fopen(STUDENTS_FILE, "rb");
    if (!fp) { printf("  [ERROR] No student records found.\n"); pauseScreen(); return; }

    Student s;
    int found = 0;
    printStudentHeader();
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.studentID == id) { printStudentRecord(&s); found = 1; break; }
    }
    fclose(fp);
    if (!found) printf("  [INFO] Student ID %d not found.\n", id);
    pauseScreen();
}

/* ============================================================
 *  ISSUE / RETURN MODULE — Implementation
 * ============================================================ */

int getNextIssueID(void) {
    FILE *fp = fopen(ISSUES_FILE, "rb");
    if (!fp) return 1;
    Issue iss;
    int maxID = 0;
    while (fread(&iss, sizeof(Issue), 1, fp) == 1) {
        if (iss.issueID > maxID) maxID = iss.issueID;
    }
    fclose(fp);
    return maxID + 1;
}

/* Returns 1 and fills 'found' if an active (unreturned) issue exists */
int isBookIssuedToStudent(int studentID, int bookID, Issue *found) {
    FILE *fp = fopen(ISSUES_FILE, "rb");
    if (!fp) return 0;
    Issue iss;
    while (fread(&iss, sizeof(Issue), 1, fp) == 1) {
        if (iss.studentID == studentID && iss.bookID == bookID && !iss.returned) {
            *found = iss;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* Add due days to a Date */
static Date addDays(Date d, int days) {
    /* Use mktime / localtime for correct month-wrap arithmetic */
    struct tm t = {0};
    t.tm_mday = d.day + days;
    t.tm_mon  = d.month - 1;
    t.tm_year = d.year - 1900;
    mktime(&t);
    Date result;
    result.day   = t.tm_mday;
    result.month = t.tm_mon + 1;
    result.year  = t.tm_year + 1900;
    return result;
}

/* ISSUE BOOK */
void issueBook(void) {
    printHeader("ISSUE BOOK");

    int stuID, bkID;
    printf("\n  Enter Student ID : ");
    if (scanf("%d", &stuID) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    if (!studentExists(stuID)) {
        printf("  [ERROR] Student ID %d not registered.\n", stuID);
        pauseScreen(); return;
    }

    printf("  Enter Book ID    : ");
    if (scanf("%d", &bkID) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    if (!bookExists(bkID)) {
        printf("  [ERROR] Book ID %d does not exist.\n", bkID);
        pauseScreen(); return;
    }

    /* Check if already issued to same student */
    Issue tmp_iss;
    if (isBookIssuedToStudent(stuID, bkID, &tmp_iss)) {
        printf("  [ERROR] This book is already issued to Student %d.\n", stuID);
        pauseScreen(); return;
    }

    /* Read the book to check availability */
    FILE *bfp = fopen(BOOKS_FILE, "rb");
    FILE *btmp = fopen("books_tmp.dat", "wb");
    if (!bfp || !btmp) {
        if (bfp)  fclose(bfp);
        if (btmp) fclose(btmp);
        printf("  [ERROR] File error.\n");
        pauseScreen(); return;
    }

    Book b;
    int bookFound = 0;
    Book issuedBook;
    memset(&issuedBook, 0, sizeof(Book));

    while (fread(&b, sizeof(Book), 1, bfp) == 1) {
        if (b.bookID == bkID) {
            bookFound = 1;
            if (b.available <= 0) {
                fclose(bfp); fclose(btmp);
                remove("books_tmp.dat");
                printf("  [ERROR] No copies of '%s' are currently available.\n", b.title);
                pauseScreen(); return;
            }
            issuedBook = b;
            b.available--;
        }
        fwrite(&b, sizeof(Book), 1, btmp);
    }
    fclose(bfp);
    fclose(btmp);

    if (!bookFound) {
        remove("books_tmp.dat");
        printf("  [ERROR] Book not found.\n");
        pauseScreen(); return;
    }

    remove(BOOKS_FILE);
    rename("books_tmp.dat", BOOKS_FILE);

    /* Create issue record */
    Issue iss;
    memset(&iss, 0, sizeof(Issue));
    iss.issueID   = getNextIssueID();
    iss.studentID = stuID;
    iss.bookID    = bkID;
    iss.issueDate = getCurrentDate();
    iss.dueDate   = addDays(iss.issueDate, BORROW_DAYS);
    iss.returned  = 0;
    iss.fine      = 0;

    FILE *ifp = fopen(ISSUES_FILE, "ab");
    if (!ifp) { printf("  [ERROR] Cannot open issues file.\n"); pauseScreen(); return; }
    fwrite(&iss, sizeof(Issue), 1, ifp);
    fclose(ifp);

    printf("\n  [SUCCESS] Book Issued!\n");
    printf("  -------------------------------------------------------\n");
    printf("  Issue ID    : %d\n", iss.issueID);
    printf("  Book        : %s (ID: %d)\n", issuedBook.title, bkID);
    printf("  Student ID  : %d\n", stuID);
    printf("  Issue Date  : "); printDate(iss.issueDate); printf("\n");
    printf("  Due Date    : "); printDate(iss.dueDate);   printf("\n");
    printf("  -------------------------------------------------------\n");
    pauseScreen();
}

/* RETURN BOOK */
void returnBook(void) {
    printHeader("RETURN BOOK");

    int stuID, bkID;
    printf("\n  Enter Student ID : ");
    if (scanf("%d", &stuID) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    printf("  Enter Book ID    : ");
    if (scanf("%d", &bkID) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    /* Find the active issue */
    Issue activeIssue;
    if (!isBookIssuedToStudent(stuID, bkID, &activeIssue)) {
        printf("  [ERROR] No active issue found for Student %d with Book %d.\n", stuID, bkID);
        pauseScreen(); return;
    }

    /* Calculate fine */
    Date today = getCurrentDate();
    long overdue = daysBetween(activeIssue.dueDate, today);
    long fine    = (overdue > 0) ? overdue * FINE_PER_DAY : 0;

    printf("\n  Issue Details:\n");
    printf("  Issue ID    : %d\n",  activeIssue.issueID);
    printf("  Issue Date  : ");     printDate(activeIssue.issueDate); printf("\n");
    printf("  Due Date    : ");     printDate(activeIssue.dueDate);   printf("\n");
    printf("  Return Date : ");     printDate(today);                 printf("\n");
    if (overdue > 0)
        printf("  Overdue Days: %ld day(s)\n", overdue);
    else
        printf("  Status      : Returned on time.\n");
    printf("  Fine Amount : Rs. %ld\n", fine);

    char confirm;
    printf("\n  Confirm return? (y/n): ");
    scanf(" %c", &confirm);
    flushInputBuffer();
    if (tolower((unsigned char)confirm) != 'y') { printf("  Return cancelled.\n"); pauseScreen(); return; }

    /* Update the issue record */
    FILE *fp  = fopen(ISSUES_FILE, "rb");
    FILE *tmp = fopen("issues_tmp.dat", "wb");
    if (!fp || !tmp) {
        if (fp)  fclose(fp);
        if (tmp) fclose(tmp);
        printf("  [ERROR] File error.\n"); pauseScreen(); return;
    }

    Issue iss;
    while (fread(&iss, sizeof(Issue), 1, fp) == 1) {
        if (iss.issueID == activeIssue.issueID) {
            iss.returned   = 1;
            iss.returnDate = today;
            iss.fine       = fine;
        }
        fwrite(&iss, sizeof(Issue), 1, tmp);
    }
    fclose(fp);
    fclose(tmp);
    remove(ISSUES_FILE);
    rename("issues_tmp.dat", ISSUES_FILE);

    /* Increment available count in book file */
    FILE *bfp  = fopen(BOOKS_FILE, "rb");
    FILE *btmp = fopen("books_tmp.dat", "wb");
    if (bfp && btmp) {
        Book b;
        while (fread(&b, sizeof(Book), 1, bfp) == 1) {
            if (b.bookID == bkID) b.available++;
            fwrite(&b, sizeof(Book), 1, btmp);
        }
        fclose(bfp); fclose(btmp);
        remove(BOOKS_FILE);
        rename("books_tmp.dat", BOOKS_FILE);
    }

    printf("\n  [SUCCESS] Book returned successfully. Fine: Rs. %ld\n", fine);
    pauseScreen();
}

/* ============================================================
 *  FINE MODULE — Implementation
 * ============================================================ */

void calculateFine(void) {
    printHeader("FINE CALCULATION");

    int stuID;
    printf("\n  Enter Student ID to view fine details: ");
    if (scanf("%d", &stuID) != 1) { flushInputBuffer(); pauseScreen(); return; }
    flushInputBuffer();

    if (!studentExists(stuID)) {
        printf("  [ERROR] Student ID %d not found.\n", stuID);
        pauseScreen(); return;
    }

    FILE *fp = fopen(ISSUES_FILE, "rb");
    if (!fp) { printf("  No issue records found.\n"); pauseScreen(); return; }

    Issue iss;
    int found = 0;
    Date today = getCurrentDate();
    long totalFine = 0;

    printf("\n  Issue records for Student ID: %d\n", stuID);
    printf("  %-8s %-8s %-12s %-12s %-12s %-8s %10s\n",
           "IssueID","BookID","Issue Date","Due Date","Return Date","Status","Fine(Rs.)");
    printLine('-', 80);

    while (fread(&iss, sizeof(Issue), 1, fp) == 1) {
        if (iss.studentID == stuID) {
            found = 1;
            long f;
            char status[12];

            if (iss.returned) {
                f = iss.fine;
                strncpy(status, "Returned", 12);
            } else {
                long overdue = daysBetween(iss.dueDate, today);
                f = (overdue > 0) ? overdue * FINE_PER_DAY : 0;
                strncpy(status, "Pending", 12);
            }

            totalFine += f;
            printf("  %-8d %-8d ", iss.issueID, iss.bookID);
            printDate(iss.issueDate); printf("  ");
            printDate(iss.dueDate);   printf("  ");
            if (iss.returned) printDate(iss.returnDate);
            else printf("  (Active) ");
            printf("  %-8s %10ld\n", status, f);
        }
    }
    fclose(fp);

    if (!found) printf("  No issue records for this student.\n");
    else {
        printLine('-', 80);
        printf("  Total Outstanding Fine: Rs. %ld\n", totalFine);
    }
    pauseScreen();
}

/* ============================================================
 *  AUTHOR DIRECTORY — Implementation
 * ============================================================ */

void displayAllAuthors(void) {
    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) { printf("\n  No book records found.\n"); return; }

    Book b;
    /* Simple approach: collect unique authors */
    char authors[500][MAX_AUTHOR];
    int  authorCount = 0;

    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        int i, dup = 0;
        for (i = 0; i < authorCount; i++) {
            if (strcasecmp(authors[i], b.author) == 0) { dup = 1; break; }
        }
        if (!dup && authorCount < 500) {
            strncpy(authors[authorCount++], b.author, MAX_AUTHOR);
        }
    }
    fclose(fp);

    if (authorCount == 0) { printf("\n  No authors found.\n"); return; }

    printf("\n  %-5s  %-40s\n", "No.", "Author Name");
    printLine('-', 50);
    int i;
    for (i = 0; i < authorCount; i++)
        printf("  %-5d  %-40s\n", i + 1, authors[i]);
    printf("\n  Total Authors: %d\n", authorCount);
}

void searchByAuthor(void) {
    printf("\n  Enter Author Name (or partial): ");
    char keyword[MAX_AUTHOR];
    fgets(keyword, MAX_AUTHOR, stdin);
    keyword[strcspn(keyword, "\n")] = '\0';

    char kl[MAX_AUTHOR];
    strncpy(kl, keyword, MAX_AUTHOR);
    int i;
    for (i = 0; kl[i]; i++) kl[i] = (char)tolower((unsigned char)kl[i]);

    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) { printf("  [ERROR] No book records found.\n"); return; }

    Book b;
    int found = 0;
    printBookHeader();
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        char al[MAX_AUTHOR];
        strncpy(al, b.author, MAX_AUTHOR);
        for (i = 0; al[i]; i++) al[i] = (char)tolower((unsigned char)al[i]);
        if (strstr(al, kl)) { printBookRecord(&b); found = 1; }
    }
    fclose(fp);
    if (!found) printf("  [INFO] No books found for author '%s'.\n", keyword);
}

void authorDirectory(void) {
    int sub;
    do {
        printHeader("AUTHOR DIRECTORY");
        printf("\n  1. Display All Authors\n");
        printf("  2. Search Books by Author\n");
        printf("  3. Back to Main Menu\n");
        printf("\n  Enter choice: ");
        if (scanf("%d", &sub) != 1) { flushInputBuffer(); break; }
        flushInputBuffer();
        switch (sub) {
            case 1: displayAllAuthors(); pauseScreen(); break;
            case 2: searchByAuthor();   pauseScreen(); break;
            case 3: break;
            default: printf("  [ERROR] Invalid choice.\n"); pauseScreen();
        }
    } while (sub != 3);
}

/* ============================================================
 *  MAIN MENU — Implementation
 * ============================================================ */

void mainMenu(void) {
    int choice;
    do {
        clearScreen();
        printLine('=', 60);
        printf("           LIBRARY MANAGEMENT SYSTEM\n");
        printLine('=', 60);
        printf("\n  BOOK MANAGEMENT\n");
        printf("    1.  Add New Book\n");
        printf("    2.  Display All Books\n");
        printf("    3.  Search Book\n");
        printf("    4.  Update Book\n");
        printf("    5.  Delete Book\n");
        printf("\n  STUDENT MANAGEMENT\n");
        printf("    6.  Register Student\n");
        printf("    7.  Display All Students\n");
        printf("    8.  Search Student\n");
        printf("\n  ISSUE & RETURN\n");
        printf("    9.  Issue Book\n");
        printf("    10. Return Book\n");
        printf("    11. Fine Details\n");
        printf("\n  OTHER\n");
        printf("    12. Author Directory\n");
        printf("    13. Exit\n");
        printf("\n");
        printLine('-', 60);
        printf("  Enter Your Choice: ");

        if (scanf("%d", &choice) != 1) {
            flushInputBuffer();
            printf("\n  [ERROR] Invalid input. Please enter a number.\n");
            pauseScreen();
            choice = 0;
            continue;
        }
        flushInputBuffer();

        switch (choice) {
            case  1: addBook();          break;
            case  2: displayAllBooks();  break;
            case  3: searchBook();       break;
            case  4: updateBook();       break;
            case  5: deleteBook();       break;
            case  6: registerStudent();  break;
            case  7: displayAllStudents(); break;
            case  8: searchStudent();    break;
            case  9: issueBook();        break;
            case 10: returnBook();       break;
            case 11: calculateFine();    break;
            case 12: authorDirectory();  break;
            case 13:
                clearScreen();
                printLine('=', 60);
                printf("  Thank you for using Library Management System!\n");
                printLine('=', 60);
                printf("\n");
                break;
            default:
                printf("\n  [ERROR] Invalid choice '%d'. Please enter 1-13.\n", choice);
                pauseScreen();
        }
    } while (choice != 13);
}

/* ============================================================
 *  MAIN FUNCTION
 * ============================================================ */

int main(void) {
    mainMenu();
    return 0;
}
