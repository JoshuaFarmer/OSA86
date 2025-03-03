#pragma once
#include <stdint.h>
#include <stdbool.h>
#define GDT_SIZE 6

struct tss_entry {
    uint32_t prev_tss;   // Previous TSS link
    uint32_t esp0;       // Kernel stack pointer
    uint32_t ss0;        // Kernel stack segment
    uint32_t esp1;       // Unused
    uint32_t ss1;        // Unused
    uint32_t esp2;       // Unused
    uint32_t ss2;        // Unused
    uint32_t cr3;        // Unused
    uint32_t eip;        // Unused
    uint32_t eflags;     // Unused
    uint32_t eax;        // Unused
    uint32_t ecx;        // Unused
    uint32_t edx;        // Unused
    uint32_t ebx;        // Unused
    uint32_t esp;        // Unused
    uint32_t ebp;        // Unused
    uint32_t esi;        // Unused
    uint32_t edi;        // Unused
    uint32_t es;         // Unused
    uint32_t cs;         // Unused
    uint32_t ss;         // Unused
    uint32_t ds;         // Unused
    uint32_t fs;         // Unused
    uint32_t gs;         // Unused
    uint32_t ldt;        // Unused
    uint16_t trap;       // Unused
    uint16_t iomap_base; // I/O Map Base Address
} __attribute__((packed));

struct gdt_entry {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t  base_middle;
        uint8_t  access;
        uint8_t  granularity;
        uint8_t  base_high;
};

struct gdt_ptr {
        uint16_t limit;
        uint32_t base;
};

struct gdt_entry gdt[GDT_SIZE];
struct gdt_ptr gdtp;

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
        gdt[num].base_low = (base & 0xFFFF);
        gdt[num].base_middle = (base >> 16) & 0xFF;
        gdt[num].base_high = (base >> 24) & 0xFF;

        gdt[num].limit_low = (limit & 0xFFFF);
        gdt[num].granularity = (limit >> 16) & 0x0F;
        gdt[num].granularity |= (gran & 0xF0);
        gdt[num].access = access;
}

void set_tss_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

struct tss_entry tss;

void init_tss() {
    // Set the kernel stack pointer (ESP0) and stack segment (SS0)
    tss.esp0 = 0x10000; // Replace with your kernel stack address
    tss.ss0 = 0x10;     // Kernel data segment selector

    // Set the I/O Map Base Address to the end of the TSS
    tss.iomap_base = sizeof(struct tss_entry);
}

void init_gdt() {
        gdtp.limit = (sizeof(struct gdt_entry) * GDT_SIZE) - 1;
        gdtp.base = (uint32_t)&gdt;
        set_gdt_entry(0, 0, 0, 0, 0);
        set_gdt_entry(1, 0x00000000, 0x01FFFFFF, 0x9A, 0xCF);
        set_gdt_entry(2, 0x00000000, 0x01FFFFFF, 0x92, 0xCF);
        set_gdt_entry(3, 0x02000000, 0x00FFFFFF, 0xFA, 0xCF);
        set_gdt_entry(4, 0x02000000, 0x00FFFFFF, 0xF2, 0xCF);
        set_tss_entry(5, (uint32_t)&tss, sizeof(struct tss_entry) - 1, 0x89, 0x40);
        asm volatile("lgdt (%0)" : : "r" (&gdtp));
        asm volatile("ltr %w0" : : "r" ((uint16_t)0x28));
        asm volatile("movl $0x10, %%eax; \
                      movl %%eax, %%ds; \
                      movl %%eax, %%es; \
                      movl %%eax, %%fs; \
                      movl %%eax, %%gs; \
                      movl %%eax, %%ss; \
                      ljmp $0x08, $goto; \
                      goto:" : : : "memory");

#ifdef VERBOSE
        puts("GDT Initialized\n");
#endif
}