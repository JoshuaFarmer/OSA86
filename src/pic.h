#ifndef PIC_H
#define PIC_H
#include <stdint.h>
#include <stdbool.h>

#define PIC1_COMMAND     0x20
#define PIC1_DATA        0x21
#define PIC2_COMMAND     0xA0
#define PIC2_DATA        0xA1

void send_eoi(uint8_t irq)
{
        if (irq >= 8)
        {
                outb(PIC2_COMMAND, 0x20);
        }
        outb(PIC1_COMMAND, 0x20);
}

void init_pic(void)
{
        /* initialize */
        outb(PIC2_DATA, 0xFF);
        outb(PIC1_COMMAND, 0x11);
        outb(PIC2_COMMAND, 0x11);
        outb(PIC1_DATA, 0x20);
        outb(PIC2_DATA, 0x28);
        outb(PIC1_DATA, 0x04);
        outb(PIC2_DATA, 0x02);
        outb(PIC1_DATA, 0x01);
        outb(PIC2_DATA, 0x01);
        outb(PIC1_DATA, 0x0);
        outb(PIC2_DATA, 0x0);
#ifdef VERBOSE
        puts("PIC Initialized\n");
#endif
        /* disable keyboard interrupts */
        uint8_t current_mask = inb(0x21);
        uint8_t new_mask = current_mask | 0x02; // IRQ1 is bit 1 (0x02)
        outb(0x21, new_mask);
}

#endif