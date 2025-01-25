#pragma once
#include <stdint.h>
#include <stdbool.h>

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

struct gdt_entry gdt[3];
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

void init_gdt() {
        gdtp.limit = (sizeof(struct gdt_entry) * 3) - 1;
        gdtp.base = (uint32_t)&gdt;

        // Null segment
        set_gdt_entry(0, 0, 0, 0, 0);  // Null descriptor

        // Kernel code segment
        set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment, read/write

        // Kernel data segment
        set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment, read/write

        // Load the GDT
        asm volatile("lgdt (%0)" : : "r" (&gdtp));

        // Set up segments
        asm volatile("movl $0x10, %%eax; \
                                  movl %%eax, %%ds; \
                                  movl %%eax, %%es; \
                                  movl %%eax, %%fs; \
                                  movl %%eax, %%gs; \
                                  movl %%eax, %%ss; \
                                  ljmp $0x08, $flush; \
                                  flush:" : : : "memory");

#ifdef VERBOSE
        puts("GDT Initialized\n");
#endif
}