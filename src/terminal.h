#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define videobuff 0xB8000
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

int TTY_WIDTH  = 80;
int TTY_HEIGHT = 25;

uint8_t termCol = 0x02;
int txtx=0, txty=0;
uint8_t Data[512];
bool active = true;

enum {
        KEY_NULL = 0,
        KEY_ESC,
        KEY_TAB = '\t',
        KEY_CTRL = 0x7F,
        KEY_ALT,
        KEY_NUML,
        KEY_SCRL,
        KEY_HOM,
        KEY_UP,
        KEY_PGU,
        KEY_LE,
        KEY_RI,
        KEY_END,
        KEY_DN,
        KEY_PGD,
        KEY_INS,
        KEY_DEL,
        
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_F11,
        KEY_F12,

        KEY_INTERACT,
} keys;

const uint16_t keyboard_map[256] = {
        KEY_NULL, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', KEY_TAB,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', KEY_CTRL, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', KEY_NULL, '#', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', KEY_ALT, '*', KEY_ALT, ' ',
        KEY_CTRL, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUML, KEY_SCRL, KEY_HOM,
        KEY_UP, KEY_PGU, '-', KEY_LE, '5', KEY_RI, '+', KEY_END, KEY_DN, KEY_PGD, KEY_INS, KEY_DEL, '\n',
        KEY_ALT, '\\', KEY_F11, KEY_F12
};

const uint16_t keyboard_map_shifted[256] = {
        KEY_NULL, KEY_ESC, '!', '"', '\\', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', KEY_TAB,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', KEY_CTRL, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '`', KEY_NULL, '#', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', KEY_ALT, '*', KEY_ALT, ' ',
        KEY_CTRL, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUML, '/', '7',
        '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', '\n',
        KEY_ALT, '\\', KEY_F11, KEY_F12
};

/*
char getc()
{
        char status;
        static int shift_pressed = 0;
        static int alt_code_mode = 0;
        static int alt_code = 0;

        status = inb(KEYBOARD_STATUS_PORT);
        if ((status & 0x01) == 0)
        {
                return -1;
        }

        unsigned char scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0x2A || scancode == 0x36) {
                shift_pressed = 1;
                return 0;
        } else if (scancode == 0xAA || scancode == 0xB6) {
                shift_pressed = 0;
                return 0;
        } else if (scancode == 0x38) {
                alt_code_mode = 1;
                alt_code = 0;
                return 0;
        } else if (scancode == 0xB8) {
                alt_code_mode = 0;
                return 0;
        }

        if (scancode & 0x80) {
                return 0;
        } else {
                if (alt_code_mode) {
                        if (keyboard_map[scancode] >= '0' && keyboard_map[scancode] <= '9') {
                                alt_code = alt_code * 10 + (keyboard_map[scancode] - '0');
                        } else if (keyboard_map[scancode] == '\n') {
                                if (alt_code >= 0 && alt_code <= 255) {
                                        alt_code_mode = 0;
                                        return (char)alt_code;
                                }
                        }
                        getc();
                        return 0;
                } else {
                        if (shift_pressed) {
                                return keyboard_map_shifted[scancode];
                        } else {
                                return keyboard_map[scancode];
                        }
                }
        }
}*/

extern volatile uint8_t CharBuff;

char getc()
{
        static int shift_pressed = 0;
        int x = CharBuff;
        CharBuff=0;

        if (x == 0x2A || x == 0x36) {
                shift_pressed = 1;
                return 0;
        } else if (x == 0xAA || x == 0xB6) {
                shift_pressed = 0;
                return 0;
        }

        if (x & 0x80) {
                return 0;
        } else {
                if (shift_pressed) {
                        return keyboard_map_shifted[x];
                } else {
                        return keyboard_map[x];
                }
        }
}

int getching = 0;

char getch()
{       
        char status;
        static int shift_pressed = 0;
        static int alt_code_mode = 0;
        static int alt_code = 0;

        status = inb(KEYBOARD_STATUS_PORT);
        do
        {
                status = inb(KEYBOARD_STATUS_PORT);
        } while ((status & 0x01) == 0);

        unsigned char scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0x2A || scancode == 0x36) {
                shift_pressed = 1;
                return 0;
        } else if (scancode == 0xAA || scancode == 0xB6) {
                shift_pressed = 0;
                return 0;
        } else if (scancode == 0x38) {
                alt_code_mode = 1;
                alt_code = 0;
                return 0;
        } else if (scancode == 0xB8) {
                alt_code_mode = 0;
                return 0;
        }

        if (scancode & 0x80) {
                return 0;
        } else {
                if (alt_code_mode) {
                        if (keyboard_map[scancode] >= '0' && keyboard_map[scancode] <= '9') {
                                alt_code = alt_code * 10 + (keyboard_map[scancode] - '0');
                        } else if (keyboard_map[scancode] == '\n') {
                                if (alt_code >= 0 && alt_code <= 255) {
                                        alt_code_mode = 0;
                                        return (char)alt_code;
                                }
                        }
                        getch();
                        return 0;
                } else {
                        if (shift_pressed) {
                                return keyboard_map_shifted[scancode];
                        } else {
                                return keyboard_map[scancode];
                        }
                }
        }
}

void scroll_cursor()
{
        char * chars = ((char*)videobuff);
        for (int y = 1; y < TTY_HEIGHT; ++y)
        {
                for (int x = 0; x < TTY_WIDTH; ++x)
                {
                        int src=((y * TTY_WIDTH) << 1) + (x << 1);
                        int dst=(((y - 1) * TTY_WIDTH) << 1) + (x << 1);
                        chars[dst]   = chars[src];
                        chars[dst+1] = chars[src+1];
                }
        }

        for (int x = 0; x < TTY_WIDTH; ++x)
        {
                int pos = (((TTY_HEIGHT - 1) * TTY_WIDTH) << 1) + (x << 1);
                chars[pos]   = '\0';
                chars[pos+1] = termCol;
        }

        if (txty >= TTY_HEIGHT)
        {
                txty = TTY_HEIGHT - 1;
        }
}

void update_cursor(const int x, const int y) {
        uint16_t pos = y * TTY_WIDTH + x;
 
        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8_t) (pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void putc(char c) {
        outb(0xE9, c);
        
        if (txty < 0) txty = 0;
        if (txtx < 0) txtx = 0;

        update_cursor(txtx, txty);

        // Backspace
        if (c == '\b') {
                --txtx;
                if (txtx < 0) {
                        txtx = 0;
                        --txty;
                        while (((char*)videobuff)[(txty * TTY_WIDTH * 2) + (txtx * 2)] != '\0') {
                                txtx++;
                        }
                }
                ((char*)videobuff)[(txty * TTY_WIDTH * 2) + (txtx * 2)] = '\0';
                ((char*)videobuff)[(txty * TTY_WIDTH * 2) + (txtx * 2) + 1] = termCol;
                update_cursor(txtx, txty);
                return;
        }

        // NULL
        if (c == 0) return;

        // Newline
        if (c == '\n') {
                txtx = 0;
                txty++;
                if (txty >= TTY_HEIGHT) {
                        scroll_cursor();
                        update_cursor(txtx, txty);
                }
                return;
        }

        if (c == '\t') {
                for (int i = 0; i < 8; ++i)
                        putc(' ');
                update_cursor(txtx, txty);
                return;
        }

        // Normal character
        ((char*)videobuff)[(txty * TTY_WIDTH * 2) + (txtx * 2)] = c;
        ((char*)videobuff)[(txty * TTY_WIDTH * 2) + (txtx * 2) + 1] = termCol;

        txtx++;
        if (txtx >= TTY_WIDTH) {
                txtx = 0;
                txty++;
                if (txty >= TTY_HEIGHT) {
                        scroll_cursor();
                }
        }

        update_cursor(txtx, txty);
}

void puts(const char* s) {
        for (int i = 0; s[i] != '\0'; i++) {
                putc(s[i]);
        }
}

void putc_at(char c, uint16_t x, uint16_t y) {
        if (x < 0) x = 0;
        if (x < 0) x = 0;
        // backspace
        if (c == '\b') {--x;
                if (x < 0) {
                        x = 0; --y;
                        while ( ((char*)videobuff)[(y*TTY_WIDTH*2) + (x*2)] != '\0') {x++;}
                }
                ((char*)videobuff)[(y*TTY_WIDTH*2)+(x*2)] = '\0';
                ((char*)videobuff)[(y*TTY_WIDTH*2)+(x*2)+1] = termCol;
                return;
        }

        // NULL
        if (c == 0 || c == '\n') return;

        // normal
        ((char*)videobuff)[(y*TTY_WIDTH*2)+(x*2)] = c;
        ((char*)videobuff)[(y*TTY_WIDTH*2)+(x*2)+1] = termCol;
}

void gets(char *buffer, int buffer_size) {
        int index = 0;
        char c;

        while (index < buffer_size - 1) {
                c = getch();

                if (c == '\n') {
                        buffer[index] = '\0';
                        putc('\n');
                        return;
                } else if (c == '\b') {
                        if (index > 0) {
                                index--;
                                putc('\b');
                                putc(' ');
                                putc('\b');
                        }
                } else if (c >= ' ' && c <= '~') {
                        buffer[index++] = c;
                        putc(c);
                }
        }

        // If the loop exits, the buffer is full
        buffer[index] = '\0'; // Null-terminate the string
        putc('\n');
}

void getsf(char *buffer, int buffer_size, uint16_t x, uint16_t y, char end) {
        int index = 0;
        char c;

        while (index < buffer_size - 1) {
                c = getch();

                if (c == end) { 
                        buffer[index] = '\0';
                        return;
                } else if (c == '\b' && index > 0) {
                        index--;
                        putc_at(' ', --x, y);
                } else if (c >= ' ' && c <= '~') {
                        buffer[index++] = c;
                        putc_at(c, x, y);
                        if (++x >= TTY_WIDTH) {
                                x = 0;
                                y++;
                        }
                } else if (c == '\n') {
                        buffer[index++] = c;
                        x = 0;
                        y++;
                }

                if (y >= TTY_HEIGHT) {
                        y = TTY_HEIGHT - 1;
                }
        }

        buffer[index] = '\0';
}

void putsn(const char* s, uint32_t n) {
        for (uint32_t i = 0; s[i] != '\0' && i < n; i++) {
                putc(s[i]);
        }
}

void puts_at(const char* s, uint16_t x, uint16_t y) {
        for (int i = 0; s[i] != '\0'; i++) {
                putc_at(s[i], x++, y);
                if (s[i] == '\n') {
                        ++y;
                        x = 0;
                }
        }
}

void PrintByte(uint8_t a) {
        char c = hchar(a>>4);
        putc(c);
        c = hchar(a&15);
        putc(c);
}

void clearScreen(uint8_t c) {
        termCol = c;
        for (int i = 0; i < TTY_WIDTH*TTY_HEIGHT*2; i++) {
                ((char*)videobuff)[i++] = '\0';
                ((char*)videobuff)[i] = c;
        }
        txtx=0;
        txty=0;
        update_cursor(0, 0);
}

int sscanf(const char *str, const char *format, ...) {
        va_list args;
        va_start(args, format);
        int count = 0;
        const char *p = format;

        while (*p) {
                if (*p == '%') {
                        p++;
                        if (*p == 'd') {
                                int *int_ptr = va_arg(args, int *);
                                int value = 0;
                                while (*str >= '0' && *str <= '9') {
                                        value = value * 10 + (*str - '0');
                                        str++;
                                }
                                *int_ptr = value;
                                count++;
                        } else if (*p == 's') {
                                char *str_ptr = va_arg(args, char *);
                                while (*str != ' ' && *str != '\0') {
                                        *str_ptr++ = *str++;
                                }
                                *str_ptr = '\0';
                                count++;
                        }
                        p++;
                } else {
                        if (*p != *str) return count;
                        p++;
                        str++;
                }
        }
        va_end(args);
        return count;
}

void put_int(int value) {
        char buffer[12];
        int i = 0;
        bool is_negative = false;

        if (value < 0) {
                is_negative = true;
                value = -value;
        }

        do {
                buffer[i++] = (value % 10) + '0';
                value /= 10;
        } while (value > 0);

        if (is_negative) {
                buffer[i++] = '-';
        }

        while (--i >= 0) {
                putc(buffer[i]);
        }
}

void put_int_at(int value, int x, int y) {
        char buffer[12];
        int i = 0;
        bool is_negative = false;

        if (value < 0) {
                is_negative = true;
                value = -value;
        }

        do {
                buffer[i++] = (value % 10) + '0';
                value /= 10;
        } while (value > 0);

        if (is_negative) {
                buffer[i++] = '-';
        }

        while (--i >= 0) {
                putc_at(buffer[i], x+i, y);
        }
}

void PRINT_DWORD_NE(int X) {
        puts("0x");
        PrintByte((X >> 24) & 255);
        PrintByte((X >> 16) & 255);
        PrintByte((X >> 8) & 255);
        PrintByte(X & 255);
}

void PRINT_DWORD(int X) {
        PRINT_DWORD_NE(X);
        putc('\n');
}

void PRINT_WORD_NE(int X) {
        puts("0x");
        PrintByte((X >> 8) & 255);
        PrintByte(X & 255);
}

void PRINT_WORD(int X) {
        PRINT_WORD_NE(X);
        putc('\n');
}

int printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        
        const char* p = format;
        while (*p) {
                if (*p == '%' && *(p + 1)) {
                        p++;
                        switch (*p) {
                                case 'c': {
                                        char c = (char) va_arg(args, int);
                                        putc(c);
                                        break;
                                }
                                case 'd': {
                                        int i = va_arg(args, int);
                                        put_int(i);
                                        break;
                                }
                                case 's': {
                                        const char* str = va_arg(args, const char*);
                                        puts(str);
                                        break;
                                }
                                case 'x': {
                                        int i = va_arg(args, int);
                                        PRINT_DWORD_NE(i);
                                        break;
                                }
                                case 'w': {
                                        int i = va_arg(args, int);
                                        PRINT_WORD_NE(i);
                                        break;
                                }
                                case 'X': {
                                        uint8_t i = va_arg(args, int);
                                        PrintByte(i);
                                        break;
                                }
                                case 'b': {
                                        uint32_t b = va_arg(args, int);
                                        for (int i = 0; i < 32; ++i)
                                        {
                                                putc(((b>>(31-i))&(1)) ? '1' : '0');
                                        }
                                        break;
                                }
                                default: {
                                        putc('%');
                                        putc(*p);
                                        break;
                                }
                        }
                } else {
                        putc(*p);
                }
                p++;
        }

        va_end(args);
        return 0;
}

#define PANIC(x, ...) void StackDump(); StackDump(); printf("PANIC: "); printf(x, ##__VA_ARGS__); while(1)