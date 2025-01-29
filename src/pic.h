#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA        0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA        0xA1

// Send End-of-Interrupt signal to the PICs
void send_eoi(uint8_t irq) {
        if (irq >= 8) {
                outb(PIC2_COMMAND, 0x20); // Send EOI to slave PIC
        }
        outb(PIC1_COMMAND, 0x20);         // Send EOI to master PIC
}

void init_pic(void) {
        outb(PIC2_DATA, 0xFF);
        outb(PIC1_COMMAND, 0x11);
        outb(PIC2_COMMAND, 0x11);
        outb(PIC1_DATA, 0x20);         // Remap IRQs 0-7 to interrupt vectors 0x20-0x27
        outb(PIC2_DATA, 0x28);         // Remap IRQs 8-15 to interrupt vectors 0x28-0x2F
        outb(PIC1_DATA, 0x04);
        outb(PIC2_DATA, 0x02);
        outb(PIC1_DATA, 0x01);
        outb(PIC2_DATA, 0x01);
        outb(PIC1_DATA, 0x0);
        outb(PIC2_DATA, 0x0);
#ifdef VERBOSE
        puts("PIC Initialized\n");
#endif
}