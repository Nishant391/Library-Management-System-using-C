#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TITLE      100
#define MAX_AUTHOR      80
#define MAX_PUBLISHER   80
#define BOOKS_FILE     "books.dat"

typedef struct {
    int    bookID;
    char   title[MAX_TITLE];
    char   author[MAX_AUTHOR];
    char   publisher[MAX_PUBLISHER];
    int    quantity;
    int    available;
} Book;

int bookExists(int id) {
    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) return 0;
    Book b;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        if (b.bookID == id) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void printAllBooks() {
    FILE *fp = fopen(BOOKS_FILE, "rb");
    if (!fp) {
        printf("No books found in %s\n", BOOKS_FILE);
        return;
    }
    Book b;
    printf("\n====================================================================================================\n");
    printf("%-6s %-32s %-25s %-20s %5s %9s\n", "ID", "Title", "Author", "Publisher", "Qty", "Available");
    printf("----------------------------------------------------------------------------------------------------\n");
    int count = 0;
    while (fread(&b, sizeof(Book), 1, fp) == 1) {
        printf("%-6d %-32s %-25s %-20s %5d %9d\n", b.bookID, b.title, b.author, b.publisher, b.quantity, b.available);
        count++;
    }
    printf("====================================================================================================\n");
    printf("Total Books in Library: %d\n\n", count);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    // If CLI arguments are passed: add a single custom book from terminal
    // Usage: ./seed_books 101 "Book Title" "Author" "Publisher" 5
    if (argc == 6) {
        int id = atoi(argv[1]);
        int qty = atoi(argv[5]);
        if (id <= 0 || qty <= 0) {
            printf("[ERROR] ID and Quantity must be positive integers.\n");
            return 1;
        }
        if (bookExists(id)) {
            printf("[WARNING] Book ID %d already exists. Skipping.\n", id);
            return 1;
        }
        Book b;
        memset(&b, 0, sizeof(Book));
        b.bookID = id;
        strncpy(b.title, argv[2], MAX_TITLE - 1);
        strncpy(b.author, argv[3], MAX_AUTHOR - 1);
        strncpy(b.publisher, argv[4], MAX_PUBLISHER - 1);
        b.quantity = qty;
        b.available = qty;

        FILE *fp = fopen(BOOKS_FILE, "ab");
        if (!fp) {
            printf("[ERROR] Cannot open %s for writing.\n", BOOKS_FILE);
            return 1;
        }
        fwrite(&b, sizeof(Book), 1, fp);
        fclose(fp);
        printf("[SUCCESS] Added book: %s (ID: %d)\n", b.title, b.bookID);
        printAllBooks();
        return 0;
    }

    // Default: Seed a curated list of books across multiple subjects
    Book sampleBooks[] = {
        {101, "The C Programming Language", "Dennis Ritchie & Brian Kernighan", "Prentice Hall", 5, 5},
        {102, "Data Structures Using C", "Reema Thareja", "Oxford University Press", 6, 6},
        {103, "Clean Code", "Robert C. Martin", "Prentice Hall", 4, 4},
        {104, "Introduction to Algorithms", "Thomas H. Cormen", "MIT Press", 8, 8},
        {105, "Operating System Concepts", "Abraham Silberschatz", "Wiley", 5, 5},
        {106, "Database System Concepts", "Abraham Silberschatz", "McGraw-Hill", 7, 7},
        {107, "Computer Networks", "Andrew S. Tanenbaum", "Pearson", 4, 4},
        {108, "Higher Engineering Mathematics", "B.S. Grewal", "Khanna Publishers", 10, 10},
        {109, "Wings of Fire", "A.P.J. Abdul Kalam", "Universities Press", 6, 6},
        {110, "To Kill a Mockingbird", "Harper Lee", "J.B. Lippincott", 3, 3}
    };

    int n = sizeof(sampleBooks) / sizeof(sampleBooks[0]);
    FILE *fp = fopen(BOOKS_FILE, "ab");
    if (!fp) {
        printf("[ERROR] Cannot open %s\n", BOOKS_FILE);
        return 1;
    }

    int added = 0;
    for (int i = 0; i < n; i++) {
        if (!bookExists(sampleBooks[i].bookID)) {
            fwrite(&sampleBooks[i], sizeof(Book), 1, fp);
            printf("[ADDED] ID %d: \"%s\" by %s\n", sampleBooks[i].bookID, sampleBooks[i].title, sampleBooks[i].author);
            added++;
        } else {
            printf("[SKIPPED] ID %d already exists.\n", sampleBooks[i].bookID);
        }
    }
    fclose(fp);

    printf("\nFinished! Added %d new book(s).\n", added);
    printAllBooks();
    return 0;
}

