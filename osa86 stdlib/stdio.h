#ifndef STDIO_H
#define STDIO_H

int __Interrupt(int eax, int ecx) // eax is res, so we can just say...
{
        __asm__ __volatile__
        (
                "movl %0, %%eax;"
                "movl $0, %%ebx;"
                "movl %1, %%ecx;"
                "int $0x80;"
                "ret"
                :
                : "r"(eax), "r"(ecx)
                : "%eax", "%ecx"
        );

        return 0;
}

enum OSA86_IC
{
        END_PROCESS,
        PUTC,
        GETC,
};

char getch(void)  { return __Interrupt(GETC,0); }
void putc(char c) { __Interrupt(PUTC,c); }

int main(void);

void init()
{
        int res = main();
        __Interrupt(END_PROCESS,res);
        while(1); // hang
}

#endif
