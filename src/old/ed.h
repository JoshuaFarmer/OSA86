#pragma once

#include <stdint.h>
#include <stddef.h>
#include "terminal.h"
#include "osafs.h"

typedef struct LChar_s {
        uint16_t value;
        struct LChar_s* next;
        struct LChar_s* prev;
} LChar_t;

LChar_t* RootChar = NULL;
LChar_t* EdTxtTail = NULL;
uint32_t EdTxtIdx = 0;

bool insertMode = false; // Track whether we're in insert mode

// Clear the screen and draw the prompt at the bottom
void drawFrame() {
        clearScreen(termCol); // Clear screen with default color

        for (LChar_t* x = RootChar; x != NULL; x = x->next) {
                putc(x->value);
        }

        // Print current index (e.g., 1)
        put_int_at(EdTxtIdx, TTY_WIDTH - 8, TTY_HEIGHT - 1);

        if (insertMode) {
                puts_at("-- INSERT --", 0, TTY_HEIGHT - 1); // Display insert mode at bottom
        } else {
                puts_at("-- DEFAULT -- ", 0, TTY_HEIGHT - 1); // Display command prompt at bottom
        }
}

LChar_t* find_last_element() {
        return EdTxtTail; // The tail is the last element
}

LChar_t* EdTxtCursor = NULL;

void add_element(uint16_t chr) {
        LChar_t* newChar = (LChar_t*)malloc(sizeof(LChar_t));
        if (newChar == NULL) {
                return; // Handle allocation failure
        }
        newChar->value = chr;
        newChar->next = NULL;
        newChar->prev = EdTxtCursor;

        if (EdTxtCursor != NULL) {
                if (EdTxtCursor->next != NULL) {
                        newChar->next = EdTxtCursor->next;
                        EdTxtCursor->next->prev = newChar;
                }
                EdTxtCursor->next = newChar;
        } else if (EdTxtTail != NULL) {
                EdTxtTail->next = newChar;
                newChar->prev = EdTxtTail;
        } else {
                RootChar = newChar; // If the list was empty
        }

        EdTxtTail = (newChar->next == NULL) ? newChar : EdTxtTail;
        EdTxtCursor = newChar; // Move cursor to new char
        EdTxtIdx++;
}

void pop_element() {
        if (EdTxtCursor == NULL) {
                return; // List is empty or cursor at start
        }

        LChar_t* to_remove = EdTxtCursor;

        if (to_remove->prev != NULL) {
                EdTxtCursor = to_remove->prev; // Move cursor left
                EdTxtCursor->next = to_remove->next;
        } else {
                // If removing the first character
                RootChar = to_remove->next;
                if (RootChar != NULL) {
                        RootChar->prev = NULL;
                }
                EdTxtCursor = NULL;
        }

        if (to_remove->next != NULL) {
                to_remove->next->prev = EdTxtCursor;
        } else {
                // If removing the last character, update the tail
                EdTxtTail = EdTxtCursor;
        }

        free(to_remove);
        EdTxtIdx--;
}

void clear_editor() {
        LChar_t* current = RootChar;
        while (current != NULL) {
                LChar_t* next = current->next;
                free(current);
                current = next;
        }
        RootChar = NULL;
        EdTxtTail = NULL;
        EdTxtIdx = 0;
}

void load(const char* buffer, uint32_t size) {
        for (uint32_t i = 0; i < size; i++) {
                add_element(buffer[i]);
        }
}

// Write text to the current block (insert mode)
void textEditorWrite() {
        insertMode = true;
        while (insertMode) {
                char c = getch();
                switch (c) {
                        case KEY_ESC:
                                insertMode = false;
                                drawFrame();
                                break;
                        case '\b': // Backspace key
                                pop_element(); // Remove the latest character
                                drawFrame();
                                break;
                        default:
                                add_element(c); // Add new character at the end
                                drawFrame();
                                break;
                }
        }
}

// Function to get the size of raw data
int raw_data_size() {
        int i = 0;
        for (LChar_t* chr = RootChar; chr != NULL; chr = chr->next,++i);
        return i;
}

void* raw_data() {
        int size = raw_data_size();
        if (size == 0) {
                return NULL;
        }

        char* data = (char*)malloc(size);
        if (data == NULL) return NULL;

        LChar_t* current = RootChar;
        for (int i = 0; i < size; i++) {
                if (current == NULL) {
                        free(data);
                        return NULL;
                }
                data[i] = (char)current->value;
                current = current->next;
                putc(data[i]);
        }
        return data;
}

// The main text editor loop
void textEditor() {
        drawFrame(); // Clear screen and display prompt
        char c[32];

        // Command loop until 'q' is pressed
        while (1) {
                c[0] = getch();

                // Command mode
                switch (c[0]) {
                        case ':':
                                puts_at(": ", 0, TTY_HEIGHT - 2);
                                getsf(c, 32, 2, TTY_HEIGHT - 2, '\n'); // Wait for user input
                                
                                char* x = strtok(c, ",");
                                char* y = strtok(NULL, "\0");

                                if (strcmp(x, "w") == 0) {                                        
                                        if (exists(y) == false) {
                                                create(y, false);
                                        }

                                        uint32_t size = raw_data_size();
                                        char* raw = raw_data();
                                        write_file(y, (unsigned char*)raw, size);
                                        free(raw);
                                } else if (strcmp(x, "e") == 0) {
                                        if (exists(y) == false) {
                                                create(y, false);
                                        }
                                        uint32_t size = file_length(y);
                                        char *buff = malloc(size);
                                        read_file(y, buff, size);
                                        buff[size] = 0;

                                        clear_editor();
                                        load(buff, size);
                                        free(buff);
                                        drawFrame();
                                }
                                break;
                        case 'q': // Quit the editor
                                return;
                        case 'i': // Enter insert mode
                                textEditorWrite();
                                drawFrame();
                                break;
                        case 'o':
                                {
                                puts_at(": ", 0, TTY_HEIGHT - 2);
                                getsf(c, 32, 2, TTY_HEIGHT - 2, '\n'); // Wait for user input
                                
                                char* y = strtok(c, " ");

                                if (exists(y) == false) {
                                        create(y, false);
                                }
                                uint32_t size = file_length(y);
                                char *buff = malloc(size);
                                read_file(y, buff, size);
                                buff[size] = 0;

                                clear_editor();
                                load(buff, size);
                                free(buff);
                                drawFrame();
                                }
                        default:
                                break;
                }
        }
}

int ed() {
        RootChar = NULL; // Initialize as an empty list
        EdTxtTail = NULL;
        EdTxtIdx = 0;
        textEditor();
        clearScreen(termCol); // Clear screen with default color
        return 0;
}
