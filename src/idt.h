#ifndef IDT_H
#define IDT_H
#define IDT_ENTRIES 256

void SystemTick();
#include <stdint.h>
#include <stdbool.h>
#include "malloc.h"
#include "osafs2.h"
#include "schedule.h"

typedef struct
{
        int code;
        int a;
        int b;
        int c;
        int d;
} SysCall;


typedef struct idt_entry
{
        uint16_t base_low;
        uint16_t selector;
        uint8_t  always0;
        uint8_t  flags;
        uint16_t base_high;
} idt_entry;

typedef struct idt_ptr
{
        uint16_t limit;
        uint32_t base;
} idt_ptr;

extern void LoadAndJump();
extern void default_exception_handler(void);
extern void OSASyscall(void);
extern void invalid_opcode_handler(void);
extern void timer_interrupt_handler(void);
extern void divide_by_zero_handler(void);
extern void keyboard_interrupt_handler(void);
extern void general_protection_fault_handler(void);
extern void page_fault_handler(void);

idt_entry idt[256];

void Exception(unsigned int addr)
{
        PANIC("Exception Error At: %x",addr);
}

void divide_by_zero()
{
        PANIC("You can't divide by zero, Silly!\n");
}

void invalid_opcode()
{
        PANIC("Invalid Opcode\n");
}

void keyboard_handler()
{
        send_eoi(1);
}

void page_fault()
{
        uint32_t faulting_address;
        __asm__ __volatile__
        (
                "movl %%cr2, %0;"
                : "=r"(faulting_address)
        );
        send_eoi(0xE);
        PANIC("Page Fault at address: %x\n", faulting_address);
}

void general_protection_fault()
{
        send_eoi(0xD);
        PANIC("General Protection Fault!\nHalting...\n");
}

void Int80(int eax, int ecx)
{
        __asm__ __volatile__
        (
                "movl %0, %%eax;"
                "movl $0, %%ebx;"
                "movl %1, %%ecx;"
                "int $0x80;"
                :
                : "r"(eax), "r"(ecx)
                : "%eax", "%ecx"
        );
}

int OSASyscallHandler(int eip, int cs, int flags, int op, int b)
{
        (void)eip;
        (void)cs;
        (void)flags;
        switch (op)
        {
                case 0:
                        MarkDead();
                        r=b;
                        break;
                case 1:
                        putc(b);
                        break;
                case 2:
                {
                        return getch();
                } break;
        }

        return 0;
}

void timer_interrupt()
{
        LookForDead();
        Scheduler();
        send_eoi(0x0);
        LoadAndJump();
        while(1);
}

void set_idt_entry(int n, uint32_t handler, struct idt_entry* idt)
{
        idt[n].base_low = handler & 0xFFFF;
        idt[n].selector = 0x08;
        idt[n].always0 = 0;
        idt[n].flags = 0x8E;
        idt[n].base_high = (handler >> 16) & 0xFFFF;
}

void load_idt(struct idt_entry* idt)
{
        static struct idt_ptr idtp;
        idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
        idtp.base = (uint32_t)idt;
        asm volatile ("lidt (%0)" : : "r" (&idtp));
}

void init_idt()
{
        cli();

        for (int i = 0; i < IDT_ENTRIES; i++)
        {
                set_idt_entry(i, (uint32_t)default_exception_handler, idt);
        }

        set_idt_entry(0x80, (uint32_t)OSASyscall, idt);
        set_idt_entry(0x20, (uint32_t)timer_interrupt_handler, idt);
        set_idt_entry(0x21, (uint32_t)keyboard_interrupt_handler, idt);
        set_idt_entry(0x00, (uint32_t)divide_by_zero_handler, idt);
        set_idt_entry(0x0E, (uint32_t)page_fault_handler, idt);
        set_idt_entry(0x0D, (uint32_t)general_protection_fault_handler, idt);
        set_idt_entry(0x06, (uint32_t)invalid_opcode_handler, idt);
        load_idt(idt);
#ifdef VERBOSE
        puts("IDT Initialized\n");
#endif
}

#endif