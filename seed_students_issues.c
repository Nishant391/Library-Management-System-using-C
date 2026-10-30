#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TITLE      100
#define MAX_AUTHOR      80
#define MAX_PUBLISHER   80
#define MAX_NAME        80
#define MAX_DEPT        50
#define MAX_CONTACT     15
#define BORROW_DAYS     14

#define BOOKS_FILE    "books.dat"
#define STUDENTS_FILE "students.dat"
#define ISSUES_FILE   "issues.dat"

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    int    bookID;
    char   title[MAX_TITLE];
    char   author[MAX_AUTHOR];
    char   publisher[MAX_PUBLISHER];
    int    quantity;
    int    available;
} Book;

typedef struct {
    int    studentID;
    char   name[MAX_NAME];
    char   department[MAX_DEPT];
    char   contact[MAX_CONTACT];
} Student;

typedef struct {
    int    issueID;
    int    studentID;
    int    bookID;
    Date   issueDate;
    Date   dueDate;
    int    returned;
    Date   returnDate;
    long   fine;
} Issue;

Date getCurrentDate(void) {
    Date d;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    d.day   = tm_info->tm_mday;
    d.month = tm_info->tm_mon + 1;
    d.year  = tm_info->tm_year + 1900;
    return d;
}

Date addDays(Date d, int days) {
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

int main(void) {
    Student students[] = {
        {1001, "Nishant Bisht",       "Computer Science",       "9876543210"},
        {1002, "Ronit Rawat",        "Information Technology", "9812345678"},
        {1003, "Rohin Verma",        "Computer Science",       "9823456789"},
        {1004, "Ashish Upadhyay",    "Electronics & Comm",     "9834567890"},
        {1005, "Nikhil Bisht",    "Mechanical Engg",        "9845678901"},
        {1006, "Nikhil Sharma",     "Computer Science",       "9856789012"},
        {1007, "Paramveer Rana",         "Electrical Engg",        "9867890123"},
        {1008, "Aman Rana",     "Civil Engg",             "9878901234"},
        {1009, "Sannidhya Tomar",         "Information Technology", "9889012345"},
        {1010, "Anuj Juyal",     "Electronics & Comm",     "9890123456"}
    };

    int bookIDsToIssue[] = {
        101,  // The C Programming Language
        102,  // Data Structures Using C
        103,  // Clean Code
        104,  // Introduction to Algorithms
        105,  // Operating System Concepts
        106,  // Database System Concepts
        107,  // Computer Networks
        108,  // Higher Engineering Mathematics
        109,  // Wings of Fire
        110    // Harry Potter
    };

    int numStudents = sizeof(students) / sizeof(students[0]);

    // 1. Write students to students.dat
    FILE *sfp = fopen(STUDENTS_FILE, "wb");
    if (!sfp) {
        printf("[ERROR] Cannot create %s\n", STUDENTS_FILE);
        return 1;
    }
    for (int i = 0; i < numStudents; i++) {
        fwrite(&students[i], sizeof(Student), 1, sfp);
    }
    fclose(sfp);
    printf("[SUCCESS] Registered %d students in %s.\n", numStudents, STUDENTS_FILE);

    // 2. Read all books from books.dat into memory so we can decrement available
    FILE *bfp = fopen(BOOKS_FILE, "rb");
    if (!bfp) {
        printf("[ERROR] Cannot open %s\n", BOOKS_FILE);
        return 1;
    }
    Book books[100];
    int bookCount = 0;
    while (fread(&books[bookCount], sizeof(Book), 1, bfp) == 1 && bookCount < 100) {
        bookCount++;
    }
    fclose(bfp);

    // 3. Create issues and decrement available count
    Date today = getCurrentDate();
    Date due = addDays(today, BORROW_DAYS);

    FILE *ifp = fopen(ISSUES_FILE, "wb");
    if (!ifp) {
        printf("[ERROR] Cannot create %s\n", ISSUES_FILE);
        return 1;
    }

    printf("\n%-8s %-10s %-20s %-8s %-32s %-12s %-12s\n",
           "IssueID", "StudentID", "Student Name", "BookID", "Book Title", "Issue Date", "Due Date");
    printf("------------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < numStudents; i++) {
        int targetBookID = bookIDsToIssue[i];
        char bookTitle[MAX_TITLE] = "Unknown";

        // Find book and decrement available
        for (int b = 0; b < bookCount; b++) {
            if (books[b].bookID == targetBookID) {
                if (books[b].available > 0) {
                    books[b].available--;
                }
                strncpy(bookTitle, books[b].title, MAX_TITLE - 1);
                break;
            }
        }

        Issue iss;
        memset(&iss, 0, sizeof(Issue));
        iss.issueID   = i + 1;
        iss.studentID = students[i].studentID;
        iss.bookID    = targetBookID;
        iss.issueDate = today;
        iss.dueDate   = due;
        iss.returned  = 0;
        iss.fine      = 0;

        fwrite(&iss, sizeof(Issue), 1, ifp);

        printf("%-8d %-10d %-20s %-8d %-32s %02d/%02d/%04d   %02d/%02d/%04d\n",
               iss.issueID, iss.studentID, students[i].name, iss.bookID,
               bookTitle, iss.issueDate.day, iss.issueDate.month, iss.issueDate.year,
               iss.dueDate.day, iss.dueDate.month, iss.dueDate.year);
    }
    fclose(ifp);

    // 4. Save updated books back to books.dat
    bfp = fopen(BOOKS_FILE, "wb");
    if (!bfp) {
        printf("[ERROR] Cannot rewrite %s\n", BOOKS_FILE);
        return 1;
    }
    for (int b = 0; b < bookCount; b++) {
        fwrite(&books[b], sizeof(Book), 1, bfp);
    }
    fclose(bfp);
    printf("\n[SUCCESS] Successfully issued 1 book to each student and updated available inventory in %s!\n", BOOKS_FILE);

    return 0;
}

