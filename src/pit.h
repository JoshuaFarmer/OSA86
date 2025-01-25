#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0        0x40  // PIT channel 0
#define PIT_COMMAND         0x43  // PIT command register

void pit_delay(unsigned int milliseconds) {
        unsigned int count = 11932 * milliseconds; // PIT frequency is approximately 11932 Hz

        // Write initial count value
        outb(count & 0xFF, PIT_CHANNEL0); // Low byte
        outb((count >> 8) & 0xFF, PIT_CHANNEL0); // High byte

        // Wait until timer reaches 0
        while (count-- > 0 );
}

#define PIT_COMMAND 0x43
#define PIT_DATA        0x40

void pit_init(uint32_t frequency) {
        // PIT runs at 1193180 Hz, calculate divisor
        uint32_t divisor = 1193180 / frequency;

        // Send command byte to PIT
        outb(PIT_COMMAND, 0x36);  // Set PIT to mode 3 (square wave)

        // Send frequency divisor
        outb(PIT_DATA, divisor & 0xFF);           // Low byte
        outb(PIT_DATA, (divisor >> 8) & 0xFF); // High byte
}