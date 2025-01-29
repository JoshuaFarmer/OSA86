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

void Int80(int code, int opa, int opb)
{
        __asm__ __volatile__
        (
                "movl %0, %%eax;"
                "movl %1, %%ebx;"
                "movl %2, %%ecx;"
                "int $0x80;"
                :
                : "r"(code), "r"(opa), "r"(opb)
                : "%eax", "%ebx", "%ecx"
        );
}

void OSASyscallHandler() {
        cli();
        int code = 0, opa = 0, opb = 0;
        __asm__ __volatile__ (
                "movl %%eax, %0;"
                "movl %%ebx, %1;"
                "movl %%ecx, %2;"
                : "=r"(code), "=r"(opa), "=r"(opb)
        );
        
        switch (code)
        {
                case 0:
                        puts("SYSCALL:");
                        putc(opa);
                        putc('\n');
                        break;
                case 1:
                        puts((const char *)opa);
                        break;
                case 2:
                        fwrite((void *)opa,1,opb,(FILE *)opb);
                        break;
                case 3:
                        fread((void *)opa,1,opb,(FILE *)opb);
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
        set_idt_entry(0x00, (uint32_t)timer_interrupt_handler, idt);
        load_idt(idt);
#ifdef VERBOSE
        puts("IDT Initialized\n");
#endif
}