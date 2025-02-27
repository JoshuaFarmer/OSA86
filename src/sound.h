#pragma once
#include <stdint.h>
#include <stdbool.h>

//Play sound using built-in speaker
static void playSound(uint32_t nFrequence) {
        uint32_t Div;
        uint8_t tmp;
 
        //Set the PIT to the desired frequency
        Div = PIT_FREQUENCY / nFrequence;
        outb(0x43, 0xb6);
        outb(0x42, (uint8_t) (Div) );
        outb(0x42, (uint8_t) (Div >> 8));
 
        //And play the sound using the PC speaker
        tmp = inb(0x61);
         if (tmp != (tmp | 3)) {
                outb(0x61, tmp | 3);
        }
}
 
//make it shut up
static void shutup() {
        uint8_t tmp = inb(0x61) & 0xFC;
 
        outb(0x61, tmp);
}

void Beep(int Freq) {
        playSound(Freq);
        delay(100);
        shutup();
}
