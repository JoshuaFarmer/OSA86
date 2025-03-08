int main();

void _start()
{
        main();
        while(1);
}

inline void outb(unsigned short port, unsigned char value)
{
        asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

int main()
{
        outb(0xE9,65);
        return 1;
}