#ifndef IO_H
#define IO_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define peekb(S, O) *(unsigned char *)(16uL * (S) + (O))
#define pokeb(S, O, V) *(unsigned char *)(16uL * (S) + (O)) = (V)
#define pokew(S, O, V) *(unsigned short *)(16uL * (S) + (O)) = (V)
#define _vmemwr(DS, DO, S, N) memcpy((char *)((DS) * 16 + (DO)), S, N)

uint16_t inw(uint16_t port)
{
        uint16_t ret;
        asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

void outw(uint16_t port, uint16_t value)
{
        asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

unsigned char inb(unsigned short port)
{
        unsigned char ret;
        asm("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

void outb(unsigned short port, unsigned char value)
{
        asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

void insw(uint16_t port, void *addr, uint32_t count)
{
        asm volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

void outl(uint16_t port, uint32_t value)
{
        asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port)
{
        uint32_t ret;
        asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

extern void ata_lba_read();

void ata_read(int lba, uint8_t size, void *p)
{
        __asm__ volatile(
            "mov %0, %%eax\n\t"
            "mov %1, %%cl\n\t"
            "mov %2, %%edi\n\t"
            "call ata_lba_read"
            :
            : "r"(lba), "r"(size), "r"(p)
            : "eax", "ecx", "edi", "memory");
}

#endif
