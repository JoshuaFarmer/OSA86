#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "malloc.h"
#include "osafs2.h"

extern void default_exception_handler(void);
extern void OSASyscall(void);

void Exception(unsigned int addr)
{
        printf("Exception Error At: %x",addr);
}

typedef struct
{
        int code;
        int a;
        int b;
        int c;
        int d;
} SysCall;

void Int80(SysCall * x)
{
        __asm__ __volatile__
        (
                "movl %0, %%eax;"
                "int $0x80;"
                :
                : "r"(x)
                : "%eax"
        );
}

void OSASyscallHandler() {
        cli();
        SysCall * args;
        __asm__ __volatile__ (
                "movl %%eax, %0;"
                : "=r"(args)
        );

        switch (args->code)
        {
                case 0:
                        putc(args->a);
                        break;
                case 1:
                        puts((const char *)args->a);
                        break;
                case 2:
                        fwrite((void *)args->a,1,args->b,(FILE *)args->c);
                        break;
                case 3:
                        fread((void *)args->a,1,args->b,(FILE *)args->c);
                        break;
        }
}

void timer_interrupt_handler() {
        send_eoi(0x20);
}

struct idt_entry {
        uint16_t base_low;         // Lower 16 bits of the handler address
        uint16_t selector;         // Kernel segment selector
        uint8_t  always0;          // This must always be zero
        uint8_t  flags;                // Flags
        uint16_t base_high;        // Upper 16 bits of the handler address
};

struct idt_ptr {
        uint16_t limit;
        uint32_t base;
};

#define IDT_ENTRIES 256

void set_idt_entry(int n, uint32_t handler, struct idt_entry* idt) {
        idt[n].base_low = handler & 0xFFFF;
        idt[n].selector = 0x08; // Kernel code segment
        idt[n].always0 = 0;
        idt[n].flags = 0x8E;        // Present, ring 0, 32-bit interrupt gate
        idt[n].base_high = (handler >> 16) & 0xFFFF;
}

void load_idt(struct idt_entry* idt) {
        static struct idt_ptr idtp;
        idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
        idtp.base = (uint32_t)idt;
        asm volatile ("lidt (%0)" : : "r" (&idtp));
}

struct idt_entry idt[256];
void init_idt() {
        cli();

        for (int i = 0; i < IDT_ENTRIES; i++) {
                set_idt_entry(i, (uint32_t)default_exception_handler, idt);
        }

        set_idt_entry(0x80, (uint32_t)OSASyscall, idt);
        set_idt_entry(0x20, (uint32_t)timer_interrupt_handler, idt);
        load_idt(idt);
#ifdef VERBOSE
        puts("IDT Initialized\n");
#endif
}