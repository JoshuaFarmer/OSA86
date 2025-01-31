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

#define GDT_SIZE 5
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

void init_gdt() {
        gdtp.limit = (sizeof(struct gdt_entry) * GDT_SIZE) - 1;
        gdtp.base = (uint32_t)&gdt;
        set_gdt_entry(0, 0, 0, 0, 0);
        set_gdt_entry(1, 0x00000000, 0x01FFFFFF, 0x9A, 0xCF);
        set_gdt_entry(2, 0x00000000, 0x01FFFFFF, 0x92, 0xCF);
        set_gdt_entry(3, 0x02000000, 0x00FFFFFF, 0xFA, 0xCF);
        set_gdt_entry(4, 0x02000000, 0x00FFFFFF, 0xF2, 0xCF);
        asm volatile("lgdt (%0)" : : "r" (&gdtp));
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