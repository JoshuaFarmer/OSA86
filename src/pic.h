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

// Initialize the PIC for Intel 80386
void init_pic(void) {
        outb(PIC2_DATA, 0xFF); // Mask all interrupts on PIC2 (slave)

        // ICW1: Start the initialization sequence (cascade mode, ICW4 required)
        outb(PIC1_COMMAND, 0x11);  // Initialize PIC1
        outb(PIC2_COMMAND, 0x11);  // Initialize PIC2

        // ICW2: Set the base interrupt vector addresses
        outb(PIC1_DATA, 0x20);         // Remap IRQs 0-7 to interrupt vectors 0x20-0x27
        outb(PIC2_DATA, 0x28);         // Remap IRQs 8-15 to interrupt vectors 0x28-0x2F

        // ICW3: Configure cascading between PIC1 and PIC2
        outb(PIC1_DATA, 0x04);         // PIC1 has a slave on IRQ2
        outb(PIC2_DATA, 0x02);         // PIC2 is connected to IRQ2 on PIC1

        // ICW4: Set PIC to 80x86 mode
        outb(PIC1_DATA, 0x01);
        outb(PIC2_DATA, 0x01);

        // OCW1: Unmask all interrupts (enable all IRQs)
        outb(PIC1_DATA, 0x0);  // Unmask all interrupts on PIC1
        outb(PIC2_DATA, 0x0);  // Unmask all interrupts on PIC2
#ifdef VERBOSE
        puts("PIC Initialized\n");
#endif
}