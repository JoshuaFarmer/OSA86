#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "malloc.h"

extern void default_exception_handler();
extern void OSASyscall();
extern void save_context();
extern void restore_context();
extern void timer_isr();

/*
        for instance

        _start:
                mov  eax, 0x10 ; command
                push eax
                mov  eax, 0x00 ; exit code
                push eax
                int 0x80
                cli                                ; never
                hlt
*/
void OSASyscallHandler() {
        int code = 0, opa = 0, opb = 0;
        __asm__ __volatile__ (
                "movl 12(%%esp), %0;"   // Get opb (3rd pushed value)
                "movl 8(%%esp), %1;"        // Get opa (2nd pushed value)
                "movl 4(%%esp), %2;"        // Get code (1st pushed value)
                : "=r"(opb), "=r"(opa), "=r"(code) // output operands
        );
        // Use code, opa, opb as needed here
}


#define MAX_TASKS 10
int current_task = 0;
struct task {
        uint32_t esp;
        uint32_t eip;
        uint32_t stack[1024]; // Each task gets its own stack
} tasks[MAX_TASKS];

// Switch to the next task
void switch_task(struct task* next_task) {
        puts("NEXT TASK IP: ");
        char str[128];
        itoa(next_task->eip, str, 10);
        puts(str);
        putc('\n');
}

// Simple round-robin scheduler
void schedule_next_task() {
        current_task = (current_task + 1) % MAX_TASKS;
        switch_task(&tasks[current_task]);
}

void timer_interrupt_handler() {
        // Acknowledge the interrupt by sending end-of-interrupt (EOI) signal to PIC
        send_eoi(0x00);

        // Call the task scheduler
        //schedule_next_task();
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
        struct idt_ptr* idtp = malloc(sizeof(struct idt_ptr));
        idtp->limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
        idtp->base = (uint32_t)&idt;
        asm volatile ("lidt (%0)" : : "r" (&idtp));
}

void init_idt() {
        init_pic();
        cli();

        struct idt_entry* idt = malloc(sizeof(struct idt_entry) * IDT_ENTRIES);

        // Set default handler for all interrupts
        for (int i = 0; i < IDT_ENTRIES; i++) {
                set_idt_entry(i, (uint32_t)default_exception_handler, idt);
        }

        set_idt_entry(0x80, (uint32_t)OSASyscall, idt);
        set_idt_entry(0x20, (uint32_t)timer_isr, idt);

        // Load the IDT
        load_idt(idt);
#ifdef VERBOSE
        puts("IDT loaded\n");
#endif
        free(idt);
}