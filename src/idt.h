#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "malloc.h"
#include "osafs2.h"

extern void default_exception_handler(void);
extern void OSASyscall(void);
extern void invalid_opcode_handler(void);
extern void timer_interrupt_handler(void);
extern void divide_by_zero_handler(void);
extern void keyboard_interrupt_handler(void);
extern void general_protection_fault_handler(void);
extern void page_fault_handler(void);

typedef struct
{
        int code;
        int a;
        int b;
        int c;
        int d;
} SysCall;

void Exception(unsigned int addr)
{
        printf("Exception Error At: %x",addr);
}

void divide_by_zero()
{
        PANIC("You can't divide by zero, Silly!\n");
}

void invalid_opcode()
{
        PANIC("Invalid Opcode\n");
}

volatile uint8_t CharBuff=0;

void keyboard_handler()
{
        if (!getching)
        {
                static uint8_t buffer[16];
                static int head = 0, tail = 0;

                uint8_t scancode = inb(KEYBOARD_DATA_PORT);
                buffer[head] = scancode;
                head = (head + 1) % 16;
                if (CharBuff == 0)
                {
                        CharBuff = buffer[tail];
                        tail = (tail + 1) % 16;
                }
        }
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

void SystemTick();

#include "schedule.h"

__attribute__((cdecl)) void timer_interrupt(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp, uint32_t esp, uint32_t eflags, uint32_t ds, uint32_t ss, uint32_t es, uint32_t fs, uint32_t gs, uint32_t eip, uint16_t cs)
{
        static int tick=0;
        if (tick & 1)
        {
                //printf("RETURN:%x\n",eip);
                //printf("EAX:%x\n",eax);
                //printf("EBX:%x\n",ebx);
                //printf("ECX:%x\n",ecx);
                //printf("EDX:%x\n",edx);
                //printf("ESI:%x\n",esi);
                //printf("EDI:%x\n",edi);
                //printf("ESP:%x\n",esp);
                //printf("EBP:%x\n",ebp);
                //printf("CS:%x\n",cs);
                //printf("DS:%x\n",ds);
                //printf("SS:%x\n",ss);
                //printf("ES:%x\n",es);
                //printf("FS:%x\n",fs);
                //printf("GS:%x\n",gs);
                //printf("EFLAGS:%b\n",eflags);
                Scheduler();
        }
        else
        {
                SystemTick();
        }
        tick++;
        send_eoi(0x0);
        return;
}

struct idt_entry {
        uint16_t base_low;         // Lower 16 bits of the handler address
        uint16_t selector;         // Kernel segment selector
        uint8_t  always0;          // This must always be zero
        uint8_t  flags;            // Flags
        uint16_t base_high;        // Upper 16 bits of the handler address
};

struct idt_ptr {
        uint16_t limit;
        uint32_t base;
};

#define IDT_ENTRIES 256

void set_idt_entry(int n, uint32_t handler, struct idt_entry* idt) {
        idt[n].base_low = handler & 0xFFFF;
        idt[n].selector = 0x08;
        idt[n].always0 = 0;
        idt[n].flags = 0x8E;
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