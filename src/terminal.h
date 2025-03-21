#ifndef TTY_H
#define TTY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60
#define TTY_TAB_WIDTH 8
#define ifsw(a,b) if (a) switch (b)

char TTY_BUFFER[160*50];
int  TTY_WIDTH,TTY_HEIGHT,TTY_X,TTY_Y,TTY_COL,TTY_XS,TTY_YS,TTY_XE,TTY_YE;

void flush()
{
        cli();
        memcpy((void*)0xB8000,TTY_BUFFER,160*50);
        sti();
}

void refresh()
{
        do
        {
                flush();
        }
        while(1);
}

enum
{
        KEY_NULL = 0,
        KEY_ESC=0x100,
        KEY_CTRL,
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
        KEY_NULL, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', KEY_CTRL, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', KEY_NULL, '#', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', KEY_ALT, '*', KEY_ALT, ' ',
        KEY_CTRL, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUML, KEY_SCRL, KEY_HOM,
        KEY_UP, KEY_PGU, '-', KEY_LE, '5', KEY_RI, '+', KEY_END, KEY_DN, KEY_PGD, KEY_INS, KEY_DEL, '\n',
        KEY_ALT, '\\', KEY_F11, KEY_F12
};

const uint16_t keyboard_map_shifted[256] = {
        KEY_NULL, KEY_ESC, '!', '"', '\\', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', KEY_CTRL, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '`', KEY_NULL, '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', KEY_ALT, '*', KEY_ALT, ' ',
        KEY_CTRL, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUML, '/', '7',
        '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', '\n',
        KEY_ALT, '\\', KEY_F11, KEY_F12
};

int printf(const char *fmt,...);

char getc()
{
        char status;
        static int shift_pressed = 0;
        static int ctrl_pressed = 0;
        static int alt_code_mode = 0;
        static int alt_code = 0;

        status = inb(KEYBOARD_STATUS_PORT);
        if ((status & 0x01) == 0)
        {
                return -1;
        }

        unsigned char scancode = inb(KEYBOARD_DATA_PORT);

        // Handle shift key
        if (scancode == 0x2A || scancode == 0x36)
        {
                shift_pressed = 1;
                return 0;
        }
        else if (scancode == 0xAA || scancode == 0xB6)
        {
                shift_pressed = 0;
                return 0;
        }

        // Handle ctrl key
        if (scancode == 0x1D)
        {
                ctrl_pressed = 1;
                return 0;
        }
        else if (scancode == 0x9D)
        {
                ctrl_pressed = 0;
                return 0;
        }

        // Handle alt key
        if (scancode == 0x38)
        {
                alt_code_mode = 1;
                alt_code = 0;
                return 0;
        }
        else if (scancode == 0xB8)
        {
                alt_code_mode = 0;
                return 0;
        }

        if (scancode & 0x80)
        {
                return 0;
        }
        else
        {
                if (alt_code_mode)
                {
                        if (keyboard_map[scancode] >= '0' && keyboard_map[scancode] <= '9')
                        {
                                alt_code = alt_code * 10 + (keyboard_map[scancode] - '0');
                        }
                        else if (keyboard_map[scancode] == '\n')
                        {
                                if (alt_code >= 0 && alt_code <= 255)
                                {
                                        alt_code_mode = 0;
                                        return (char)alt_code;
                                }
                        }
                        getc();
                        return 0;
                }
                else
                {
                        char key = shift_pressed ? keyboard_map_shifted[scancode] : keyboard_map[scancode];

                        // Handle Ctrl key combinations
                        if (ctrl_pressed)
                        {
                                if (key >= 'a' && key <= 'z')
                                {
                                        return key - 'a' + 1;  // Ctrl+A -> 1, Ctrl+B -> 2, ..., Ctrl+Z -> 26
                                }
                                else if (key >= 'A' && key <= 'Z')
                                {
                                        return key - 'A' + 1;  // Ctrl+A -> 1, Ctrl+B -> 2, ..., Ctrl+Z -> 26
                                }
                        }

                        return key;
                }
        }
}

char getch()
{
        char status;
        static int shift_pressed = 0;
        static int alt_code_mode = 0;
        static int alt_code = 0;
        static int ctrl_pressed = 0;

        status = inb(KEYBOARD_STATUS_PORT);
        do
        {
                status = inb(KEYBOARD_STATUS_PORT);
        }
        while ((status & 0x01) == 0);

        unsigned char scancode = inb(KEYBOARD_DATA_PORT);

        // Handle shift key
        if (scancode == 0x2A || scancode == 0x36)
        {
                shift_pressed = 1;
                return 0;
        }
        else if (scancode == 0xAA || scancode == 0xB6)
        {
                shift_pressed = 0;
                return 0;
        }

        // Handle ctrl key
        if (scancode == 0x1D)
        {
                ctrl_pressed = 1;
                return 0;
        }
        else if (scancode == 0x9D)
        {
                ctrl_pressed = 0;
                return 0;
        }

        // Handle alt key
        if (scancode == 0x38)
        {
                alt_code_mode = 1;
                alt_code = 0;
                return 0;
        }
        else if (scancode == 0xB8)
        {
                alt_code_mode = 0;
                return 0;
        }

        if (scancode & 0x80)
        {
                return 0;
        }
        else
        {
                if (alt_code_mode)
                {
                        if (keyboard_map[scancode] >= '0' && keyboard_map[scancode] <= '9')
                        {
                                alt_code = alt_code * 10 + (keyboard_map[scancode] - '0');
                        }
                        else if (keyboard_map[scancode] == '\n')
                        {
                                if (alt_code >= 0 && alt_code <= 255)
                                {
                                        alt_code_mode = 0;
                                        return (char)alt_code;
                                }
                        }
                        getc();
                        return 0;
                }
                else
                {
                        char key = shift_pressed ? keyboard_map_shifted[scancode] : keyboard_map[scancode];

                        // Handle Ctrl key combinations
                        if (ctrl_pressed)
                        {
                                if (key >= 'a' && key <= 'z')
                                {
                                        return key - 'a' + 1;  // Ctrl+A -> 1, Ctrl+B -> 2, ..., Ctrl+Z -> 26
                                }
                                else if (key >= 'A' && key <= 'Z')
                                {
                                        return key - 'A' + 1;  // Ctrl+A -> 1, Ctrl+B -> 2, ..., Ctrl+Z -> 26
                                }
                        }

                        return key;
                }
        }
}

void ScrollScreen()
{
        for (int y = TTY_YS+1; y < TTY_YE; ++y)
        {
                for (int x = TTY_XS; x < TTY_XE; ++x)
                {
                        int src=((y * TTY_WIDTH) << 1) + (x << 1);
                        int dst=(((y - 1) * TTY_WIDTH) << 1) + (x << 1);
                        TTY_BUFFER[dst]   = TTY_BUFFER[src];
                        TTY_BUFFER[dst+1] = TTY_BUFFER[src+1];
                }
        }

        for (int x = TTY_XS; x < TTY_XE; ++x)
        {
                int pos1 = (((TTY_YE - 1) * TTY_WIDTH) << 1) + (x << 1);
                TTY_BUFFER[pos1]   = '\0';
                TTY_BUFFER[pos1+1] = TTY_COL;
        }

        if (TTY_Y >= TTY_YE)
        {
                TTY_Y = TTY_YE - 1;
        }
}

void update_cursor(const int x, const int y)
{
        uint16_t pos = y * TTY_WIDTH + x;
        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8_t) (pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void putc(char c)
{
        outb(0xE9,c);
        if (TTY_Y < TTY_XS) TTY_Y = TTY_XS;
        if (TTY_X < TTY_YS) TTY_X = TTY_YS;
        update_cursor(TTY_X, TTY_Y);

        switch (c)
        {
                case '\b':
                {
                        --TTY_X;
                        if (TTY_X < TTY_YS && TTY_Y >= TTY_YS)
                        {
                                TTY_X=TTY_XS,--TTY_Y;
                                for (;((char*)TTY_BUFFER)[(TTY_Y * TTY_WIDTH * 2) + (TTY_X * 2)] != '\0';++TTY_X);
                        }
                        ((char*)TTY_BUFFER)[(TTY_Y * TTY_WIDTH * 2) + (TTY_X * 2)] = '\0';
                        ((char*)TTY_BUFFER)[(TTY_Y * TTY_WIDTH * 2) + (TTY_X * 2) + 1] = TTY_COL;
                        update_cursor(TTY_X, TTY_Y);
                } break;
                case 0:
                        break;
                case '\n':
                {
                        TTY_X=TTY_XS;
                        (++TTY_Y >= TTY_YE) ? ScrollScreen(),update_cursor(TTY_X, TTY_Y) : 0;
                } break;
                case '\t':
                {
                        if ((TTY_X % TTY_TAB_WIDTH) == 0) putc(' ');
                        while ((TTY_X % TTY_TAB_WIDTH) != 0)
                                putc(' ');
                        update_cursor(TTY_X, TTY_Y);
                        return;
                }
                default:
                {
                        ((char*)TTY_BUFFER)[(TTY_Y * TTY_WIDTH * 2) + (TTY_X * 2)] = c;
                        ((char*)TTY_BUFFER)[(TTY_Y * TTY_WIDTH * 2) + (TTY_X * 2) + 1] = TTY_COL;
                        ++TTY_X;
                        (TTY_X >= TTY_XE) ? TTY_X=TTY_XS,++TTY_Y,(TTY_Y >= TTY_YE) ? ScrollScreen() : 0 : 0;
                        update_cursor(TTY_X, TTY_Y);
                } break;
        }
}

void puts(s)
        const char * s;
{
        while(*s)putc(*(s++));
}

void putc_at(char c, uint16_t x, uint16_t y)
{
        int tx=TTY_X;
        int ty=TTY_Y;
        TTY_X=x;
        TTY_Y=y;
        putc(c);
        TTY_X=tx;
        TTY_Y=ty;
}

void gets(b,s)
        char * b;
        int s;
{
        int i = 0;
        char c;

        while (i < s - 1)
        {
                c = getch();
                if (c == '\n')
                {
                        b[i] = '\0';
                        putc('\n');
                        return;
                }
                else if (c == '\b' && i > 0)
                {
                        --i;
                        putc('\b');
                        putc(' ');
                        putc('\b');
                }
                else if (c >= ' ' && c <= '~')
                {
                        b[i++] = c;
                        putc(c);
                }
        }

        b[i] = '\0';
        putc('\n');
}

void getsf(b,s,x,y,end)
        char * b,end;
        int s;
        uint16_t x,y;
{
        int i = 0;
        char c;

        while (i < s - 1)
        {
                c = getch();

                if (c == end)
                { 
                        b[i] = '\0';
                        return;
                }
                else if (c == '\b' && i > 0)
                {
                        --i;
                        putc_at(' ', --x, y);
                }
                else if (c >= ' ' && c <= '~')
                {
                        b[i++] = c;
                        putc_at(c, x, y);
                        if (++x >= TTY_WIDTH)
                        {
                                x = 0;
                                y++;
                        }
                }
                else if (c == '\n')
                {
                        b[i++] = c;
                        x = 0;
                        y++;
                }

                if (y >= TTY_HEIGHT)
                {
                        y = TTY_HEIGHT - 1;
                }
        }

        b[i] = '\0';
}

int putsn(s,n)
        const char * s;
        int n;
{
        int c=0;
        while(*s&&--n)putc(*(s++)),++c;
        return c;
}

void puts_at(s,x,y)
        const char* s;
        int x,y;
{
        int tx=TTY_X;
        int ty=TTY_Y;
        TTY_X=x;
        TTY_Y=y;
        puts(s);
        TTY_X=tx;
        TTY_Y=ty;
}

void PrintByte(uint8_t a)
{
        char c = hchar(a>>4);
        putc(c);
        c = hchar(a&15);
        putc(c);
}

void clearScreen(uint8_t c)
{
        TTY_COL = c;
        for (int i = 0; i < TTY_WIDTH*TTY_HEIGHT*2; i++)
        {
                ((char*)TTY_BUFFER)[i++] = '\0';
                ((char*)TTY_BUFFER)[i] = c;
        }

        TTY_X=TTY_XS;
        TTY_Y=TTY_YS;
        update_cursor(TTY_X, TTY_Y);
}

int sscanf(const char *str, const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        int count = 0;
        const char *p = fmt;

        while (*p)
        {
                if (*p == '%')
                {
                        p++;
                        if (*p == 'd')
                        {
                                int *int_ptr = va_arg(args, int *);
                                int value = 0;
                                while (*str >= '0' && *str <= '9') {
                                        value = value * 10 + (*str - '0');
                                        str++;
                                }
                                *int_ptr = value;
                                count++;
                        }
                        else if (*p == 's')
                        {
                                char *str_ptr = va_arg(args, char *);
                                while (*str != ' ' && *str != '\0') {
                                        *str_ptr++ = *str++;
                                }
                                *str_ptr = '\0';
                                count++;
                        }
                        p++;
                }
                else
                {
                        if (*p != *str) return count;
                        p++;
                        str++;
                }
        }
        va_end(args);
        return count;
}

void put_int(int value)
{
        char b[32];
        int i = 0;
        bool is_negative = false;

        if (value < 0)
        {
                is_negative = true;
                value = -value;
        }

        do
        {
                b[i++] = (value % 10) + '0';
                value /= 10;
        }
        while (value > 0);

        if (is_negative)
        {
                b[i++] = '-';
        }

        while (--i >= 0)
        {
                putc(b[i]);
        }
}

void put_int_at(int value, int x, int y)
{
        char b[12];
        int i = 0;
        bool is_negative = false;

        if (value < 0)
        {
                is_negative = true;
                value = -value;
        }

        do
        {
                b[i++] = (value % 10) + '0';
                value /= 10;
        }
        while (value > 0);

        if (is_negative)
        {
                b[i++] = '-';
        }

        while (--i >= 0)
        {
                putc_at(b[i], x+i, y);
        }
}

void PRINT_DWORD_NE(int X)
{
        puts("0x");
        PrintByte((X >> 24) & 255);
        PrintByte((X >> 16) & 255);
        PrintByte((X >> 8) & 255);
        PrintByte(X & 255);
}

void PRINT_DWORD(int X)
{
        PRINT_DWORD_NE(X);
        putc('\n');
}

void PRINT_WORD_NE(int X)
{
        puts("0x");
        PrintByte((X >> 8) & 255);
        PrintByte(X & 255);
}

void PRINT_WORD(int X)
{
        PRINT_WORD_NE(X);
        putc('\n');
}

#define perror(fmt, ...) printf(fmt, ##__VA_ARGS__)

typedef struct
{
        int min_x;
        int min_y;
        int max_y;
        int max_x;
        int width;
        int height;
        int colour;
} TTY_STATE;

static TTY_STATE tty[64];
static uint8_t   ttyp;

void PushTTYState()
{
        tty[ttyp].max_x  = TTY_XE;
        tty[ttyp].max_y  = TTY_YE;
        tty[ttyp].min_x  = TTY_XS;
        tty[ttyp].min_y  = TTY_YS;
        tty[ttyp].width  = TTY_WIDTH;
        tty[ttyp].height = TTY_HEIGHT;
        tty[ttyp].colour = TTY_COL;
        ttyp = (ttyp + 1) % 64;
}

void PopTTYState()
{
        ttyp = (ttyp - 1) % 64;
        TTY_XE     = tty[ttyp].max_x;
        TTY_YE     = tty[ttyp].max_y;
        TTY_XS     = tty[ttyp].min_x;
        TTY_YS     = tty[ttyp].min_y;
        TTY_WIDTH  = tty[ttyp].width;
        TTY_HEIGHT = tty[ttyp].height;
        TTY_COL    = tty[ttyp].colour;
}

int printf(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        PushTTYState();
        const char* p = fmt;
        while (*p)
        {
                if (*p == '%' && *(p + 1))
                {
                        p++;
                        switch (*p)
                        {
                                case 'c':
                                {
                                        char c = (char) va_arg(args, int);
                                        putc(c);
                                        break;
                                }
                                case 'd':
                                {
                                        int i = va_arg(args, int); 
                                        put_int(i);
                                        break;
                                }
                                case 's':
                                {
                                        const char* str = va_arg(args, const char*);
                                        puts(str);
                                        break;
                                }
                                case 'x':
                                {
                                        int i = va_arg(args, int);
                                        PRINT_DWORD_NE(i);
                                        break;
                                }
                                case 'w':
                                {
                                        int i = va_arg(args, int);
                                        PRINT_WORD_NE(i);
                                        break;
                                }
                                case 'X':
                                {
                                        uint8_t i = va_arg(args, int);
                                        PrintByte(i);
                                        break;
                                }
                                case 'b':
                                {
                                        uint32_t b = va_arg(args, int);
                                        for (int i = 0; i < 32; ++i)
                                        {
                                                putc(((b>>(31-i))&(1)) ? '1' : '0');
                                        }
                                        break;
                                }
                                default:
                                {
                                        putc('%');
                                        putc(*p);
                                        break;
                                }
                        }
                }
                else if ((uint8_t)*p == 255)
                {
                        int n=0;
                        while (*(p) != ']')
                        {
                                if (*p >= '0' && *p <= '9')
                                {
                                        n = (n * 10) + (*p - '0');
                                }
                                else if (*p == 'c')
                                {
                                        TTY_COL = n;
                                        n = 0;
                                }
                                else if (*p == 'b')
                                {
                                        TTY_COL &= 0xF;
                                        TTY_COL |= (n<<4);
                                        n = 0;
                                }
                                else if (*p == 'f')
                                {
                                        TTY_COL &= 0xF0;
                                        TTY_COL |= n;
                                        n = 0;
                                }
                                else if (*p == 'x')
                                {
                                        TTY_X = n;
                                        n = 0;
                                }
                                else if (*p == 'y')
                                {
                                        TTY_Y = n;
                                        n = 0;
                                }
                                else if (*p == 'r')
                                {
                                        PopTTYState();
                                        PushTTYState();
                                        n = 0;
                                }
                                else if (*p == '*')
                                {
                                        n = va_arg(args, int);
                                }
                                ++p;
                        }
                }
                else
                {
                        putc(*p);
                }
                p++;
        }
        PopTTYState();
        va_end(args);
        return 0;
}

void init_tty()
{
        /* set system defaults */
        TTY_COL    = 0x07;
        TTY_WIDTH  = 80;
        TTY_HEIGHT = 25;
        TTY_XS     = 1;
        TTY_YS     = 1;
        TTY_XE     = TTY_WIDTH-1;
        TTY_YE     = TTY_HEIGHT-1;
        clearScreen(TTY_COL);
}

#define PANIC(x, ...) void StackDump(); StackDump(); printf("PANIC: "); printf(x, ##__VA_ARGS__); while(1)

void mode();

#endif
