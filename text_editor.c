#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#ifdef _WIN32
#include <conio.h> // For Windows input handling
#else
#include <ncurses.h> // For Linux/macOS input handling
#include <termios.h> // For terminal settings
#endif

#define INITIAL_BUFFER_SIZE 1024
#define MAX_LINES 1000

// Dynamic text buffer
typedef struct {
    char *lines[MAX_LINES]; // Array of lines
    size_t lineCount;       // Number of lines
    size_t lineLengths[MAX_LINES]; // Length of each line
} TextBuffer;

char currentFilename[100] = ""; // Store the current filename

void initTextBuffer(TextBuffer *buffer) {
    buffer->lineCount = 0;
    for (int i = 0; i < MAX_LINES; i++) {
        buffer->lines[i] = NULL;
        buffer->lineLengths[i] = 0;
    }
}

void freeTextBuffer(TextBuffer *buffer) {
    for (int i = 0; i < buffer->lineCount; i++) {
        if (buffer->lines[i] != NULL) {
            free(buffer->lines[i]);
        }
    }
    buffer->lineCount = 0;
}

void appendLineToTextBuffer(TextBuffer *buffer, const char *text) {
    if (buffer->lineCount >= MAX_LINES) {
        fprintf(stderr, "Error: Maximum number of lines reached.\n");
        return;
    }

    size_t textLength = strlen(text);
    buffer->lines[buffer->lineCount] = (char *)malloc(textLength + 1);
    if (buffer->lines[buffer->lineCount] == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(1);
    }
    strcpy(buffer->lines[buffer->lineCount], text);
    buffer->lineLengths[buffer->lineCount] = textLength;
    buffer->lineCount++;
}

void ensureLineExists(TextBuffer *buffer, int lineIndex) {
    if (lineIndex >= buffer->lineCount) {
        // Add empty lines until the desired line exists
        while (buffer->lineCount <= lineIndex) {
            appendLineToTextBuffer(buffer, "");
        }
    }
}

void openFile(TextBuffer *buffer) {
    printf("Enter the file name to open: ");
    scanf("%s", currentFilename);
    getchar(); // Clear the input buffer

    FILE *file = fopen(currentFilename, "r");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", currentFilename);
        return;
    }

    // Clear the buffer before loading new content
    freeTextBuffer(buffer);
    initTextBuffer(buffer);

    // Read the entire file into the buffer
    char line[INITIAL_BUFFER_SIZE];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove the newline character (if any)
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        appendLineToTextBuffer(buffer, line);
    }
    fclose(file);

    // If the file is empty, initialize at least one line
    if (buffer->lineCount == 0) {
        appendLineToTextBuffer(buffer, "");
    }

    printf("\nFile content:\n");
    for (int i = 0; i < buffer->lineCount; i++) {
        printf("%s\n", buffer->lines[i]);
    }
    printf("File loaded successfully.\n");
}

void saveFile(TextBuffer *buffer) {
    if (strlen(currentFilename) == 0) {
        printf("Error: No file is open. Please open or create a file first.\n");
        return;
    }

    FILE *file = fopen(currentFilename, "w");
    if (file == NULL) {
        printf("Error: Could not save file '%s'.\n", currentFilename);
        return;
    }

    for (int i = 0; i < buffer->lineCount; i++) {
        fprintf(file, "%s\n", buffer->lines[i]);
    }
    fclose(file);

    printf("File saved successfully.\n");
}

void createFile() {
    printf("Enter the name of the new file: ");
    scanf("%s", currentFilename);
    getchar(); // Clear the input buffer

    FILE *file = fopen(currentFilename, "w");
    if (file == NULL) {
        printf("Error: Could not create file '%s'.\n", currentFilename);
        return;
    }
    fclose(file);

    // Initialize the buffer with an empty line
    TextBuffer buffer;
    initTextBuffer(&buffer);
    appendLineToTextBuffer(&buffer, "");

    printf("New file created successfully.\n");
}

#ifdef _WIN32
void editText(TextBuffer *buffer) {
    int cursorX = 0, cursorY = 0;
    int ch;

    char *left = NULL;
    char *right = NULL;

    printf("\nEntering edit mode. Use arrow keys to navigate. Press ESC to exit.\n");
    while (1) {
        // Display the text
        printf("\n");
        for (int i = 0; i < buffer->lineCount; i++) {
            printf("%s\n", buffer->lines[i]);
        }

        // Move the cursor
        gotoxy(cursorX + 1, cursorY + 1); // Move cursor to (x, y)

        // Get input
        ch = _getch();
        if (ch == 0 || ch == 224) { // Arrow keys generate two codes
            switch (_getch()) {
                case 72: // Up
                    if (cursorY > 0) cursorY--;
                    break;
                case 80: // Down
                    if (cursorY < buffer->lineCount - 1) cursorY++;
                    break;
                case 75: // Left
                    if (cursorX > 0) cursorX--;
                    break;
                case 77: // Right
                    if (cursorX < buffer->lineLengths[cursorY]) cursorX++;
                    break;
            }
        } else if (ch == 8) { // Backspace
            if (cursorX > 0) {
                memmove(buffer->lines[cursorY] + cursorX - 1, buffer->lines[cursorY] + cursorX, buffer->lineLengths[cursorY] - cursorX + 1);
                buffer->lineLengths[cursorY]--;
                cursorX--;
            }
        } else if (ch == 13) { // Enter key (Windows)
            char *left = (char *)malloc(cursorX + 1);
            char *right = (char *)malloc(buffer->lineLengths[cursorY] - cursorX + 1);

            // Split the current line at the cursor position
            strncpy(left, buffer->lines[cursorY], cursorX);
            left[cursorX] = '\0';
            strcpy(right, buffer->lines[cursorY] + cursorX);

            // Update the current line
            free(buffer->lines[cursorY]);
            buffer->lines[cursorY] = left;
            buffer->lineLengths[cursorY] = cursorX;

            // Insert the new line
            for (int i = buffer->lineCount; i > cursorY + 1; i--) {
                buffer->lines[i] = buffer->lines[i - 1];
                buffer->lineLengths[i] = buffer->lineLengths[i - 1];
            }
            buffer->lines[cursorY + 1] = right;
            buffer->lineLengths[cursorY + 1] = strlen(right);
            buffer->lineCount++;

            // Move the cursor to the beginning of the new line
            cursorY++;
            cursorX = 0;
        } else if (ch == 19) { // Ctrl+S to save
            saveFile(buffer);
        } else if (ch == 27) { // ESC key to exit edit mode
            return;
        } else if (ch >= 32 && ch <= 126) { // Printable ASCII characters
            // Ensure the line exists
            ensureLineExists(buffer, cursorY);

            // Ensure the line is long enough
            if (cursorX > buffer->lineLengths[cursorY]) {
                // Pad the line with spaces
                buffer->lines[cursorY] = (char *)realloc(buffer->lines[cursorY], cursorX + 2);
                memset(buffer->lines[cursorY] + buffer->lineLengths[cursorY], ' ', cursorX - buffer->lineLengths[cursorY]);
                buffer->lines[cursorY][cursorX] = '\0';
                buffer->lineLengths[cursorY] = cursorX;
            }

            // Insert character at cursor position
            memmove(buffer->lines[cursorY] + cursorX + 1, buffer->lines[cursorY] + cursorX, buffer->lineLengths[cursorY] - cursorX + 1);
            buffer->lines[cursorY][cursorX] = ch;
            buffer->lineLengths[cursorY]++;
            cursorX++;
        }
    }
}
#else
void editText(TextBuffer *buffer) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Save old terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO | ISIG); // Disable canonical mode, echo, and signals
    newt.c_iflag &= ~(IXON); // Disable flow control (Ctrl+S/Ctrl+Q)
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new terminal settings

    initscr(); // Initialize ncurses
    cbreak();  // Disable line buffering
    noecho();  // Don't echo input
    keypad(stdscr, TRUE); // Enable arrow keys

    int cursorX = 0, cursorY = 0;
    int ch;

    char *left = NULL;
    char *right = NULL;

    while (1) {
        // Display the text
        clear();
        for (int i = 0; i < buffer->lineCount; i++) {
            printw("%s\n", buffer->lines[i]);
        }
        move(cursorY, cursorX); // Move the cursor to the current position
        refresh();

        // Get input
        ch = getch();
        switch (ch) {
            case KEY_UP:
                if (cursorY > 0) cursorY--;
                break;
            case KEY_DOWN:
                if (cursorY < buffer->lineCount - 1) cursorY++;
                break;
            case KEY_LEFT:
                if (cursorX > 0) cursorX--;
                break;
            case KEY_RIGHT:
                if (cursorX < buffer->lineLengths[cursorY]) cursorX++;
                break;
            case 127: // Backspace (Unix-like systems)
            case 8:   // Backspace (Windows)
                if (cursorX > 0) {
                    memmove(buffer->lines[cursorY] + cursorX - 1, buffer->lines[cursorY] + cursorX, buffer->lineLengths[cursorY] - cursorX + 1);
                    buffer->lineLengths[cursorY]--;
                    cursorX--;
                }
                break;
            case 10: {// Enter key (Unix-like systems)
                char *left = (char *)malloc(cursorX + 1);
                char *right = (char *)malloc(buffer->lineLengths[cursorY] - cursorX + 1);

                // Split the current line at the cursor position
                strncpy(left, buffer->lines[cursorY], cursorX);
                left[cursorX] = '\0';
                strcpy(right, buffer->lines[cursorY] + cursorX);

                // Update the current line
                free(buffer->lines[cursorY]);
                buffer->lines[cursorY] = left;
                buffer->lineLengths[cursorY] = cursorX;

                // Insert the new line
                for (int i = buffer->lineCount; i > cursorY + 1; i--) {
                    buffer->lines[i] = buffer->lines[i - 1];
                    buffer->lineLengths[i] = buffer->lineLengths[i - 1];
                }
                buffer->lines[cursorY + 1] = right;
                buffer->lineLengths[cursorY + 1] = strlen(right);
                buffer->lineCount++;

                // Move the cursor to the beginning of the new line
                cursorY++;
                cursorX = 0;
                break;
            }
            case 19: // Ctrl+S to save
                saveFile(buffer);
                break;
            case 27: // ESC key to exit edit mode
                endwin();
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old terminal settings
                return;
            default:
                // Ensure the line exists
                ensureLineExists(buffer, cursorY);

                // Ensure the line is long enough
                if (cursorX > buffer->lineLengths[cursorY]) {
                    // Pad the line with spaces
                    buffer->lines[cursorY] = (char *)realloc(buffer->lines[cursorY], cursorX + 2);
                    memset(buffer->lines[cursorY] + buffer->lineLengths[cursorY], ' ', cursorX - buffer->lineLengths[cursorY]);
                    buffer->lines[cursorY][cursorX] = '\0';
                    buffer->lineLengths[cursorY] = cursorX;
                }

                // Insert character at cursor position
                memmove(buffer->lines[cursorY] + cursorX + 1, buffer->lines[cursorY] + cursorX, buffer->lineLengths[cursorY] - cursorX + 1);
                buffer->lines[cursorY][cursorX] = ch;
                buffer->lineLengths[cursorY]++;
                cursorX++;
                break;
        }
    }

    endwin(); // End ncurses
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old terminal settings
}
#endif

int main() {
    TextBuffer buffer;
    initTextBuffer(&buffer);

    int choice;
    while (1) {
        printf("\n--- Text Editor ---\n");
        printf("1. Open File\n");
        printf("2. Create New File\n");
        printf("3. Edit Text (Press Ctrl+S to save the file. Press Esc to close the editor)\n");
        printf("4. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Clear the input buffer

        switch (choice) {
            case 1:
                openFile(&buffer);
                break;
            case 2:
                createFile();
                break;
            case 3:
                editText(&buffer);
                break;
            case 4:
                printf("Exiting the text editor. Goodbye!\n");
                freeTextBuffer(&buffer);
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}