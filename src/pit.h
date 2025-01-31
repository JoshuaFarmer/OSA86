#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0        0x40  // PIT channel 0
#define PIT_COMMAND         0x43  // PIT command register
#define PIT_STATUS   0x61

void delay(unsigned int milliseconds) {
        cli();
        unsigned int count = 11932 * milliseconds;
        outb(count & 0xFF, PIT_CHANNEL0);
        outb((count >> 8) & 0xFF, PIT_CHANNEL0);
        outb(0x36, PIT_COMMAND);
        while ((inb(PIT_STATUS) & 0x80) == 0)
        {
        }
        sti();
}

#define PIT_COMMAND 0x43
#define PIT_DATA        0x40

void init_pit(uint32_t frequency) {
        uint32_t divisor = 1193180 / frequency;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_DATA, divisor & 0xFF);
        outb(PIT_DATA, (divisor >> 8) & 0xFF);
        printf("PIT Initialized\n");
}
