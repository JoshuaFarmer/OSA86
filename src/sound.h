#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <stdbool.h>
#include "pit.h"

static void playSound(uint32_t nFrequence)
{
        uint32_t Div;
        uint8_t tmp;
 
        Div = PIT_FREQUENCY / nFrequence;
        outb(0x43, 0xb6);
        outb(0x42, (uint8_t) (Div) );
        outb(0x42, (uint8_t) (Div >> 8));
 
        tmp = inb(0x61);
        if (tmp != (tmp | 3))
        {
                outb(0x61, tmp | 3);
        }
}
 
//make it shut up
static void shutup()
{
        uint8_t tmp = inb(0x61) & 0xFC;
 
        outb(0x61, tmp);
}

void Beep(int Freq)
{
        playSound(Freq);
        delay(100);
        shutup();
}

#endif