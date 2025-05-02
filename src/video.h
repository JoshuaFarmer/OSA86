#ifndef VIDEO_H
#define VIDEO_H

#include "io.h"
#include "font.h"

#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_INSTAT_READ 0x3DA
#define VGA_NUM_SEQ_REGS 0x005
#define VGA_NUM_CRTC_REGS 0x019
#define VGA_NUM_GC_REGS 0x009
#define VGA_NUM_AC_REGS 0x015
#define VGA_NUM_REGS (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
#define VGA__DAC_ADDR_W 0x3c8
#define VGA__DAC_DATA 0x3c9

uint8_t g_320x200x256[] =
    {
        0x63,
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00};

void read_regs(uint8_t *regs)
{
        uint32_t i;
        *regs = inb(VGA_MISC_READ);
        regs++;
        for (i = 0; i < VGA_NUM_SEQ_REGS; i++)
        {
                outb(VGA_SEQ_INDEX, i);
                *regs = inb(VGA_SEQ_DATA);
                regs++;
        }
        for (i = 0; i < VGA_NUM_CRTC_REGS; i++)
        {
                outb(VGA_CRTC_INDEX, i);
                *regs = inb(VGA_CRTC_DATA);
                regs++;
        }
        for (i = 0; i < VGA_NUM_GC_REGS; i++)
        {
                outb(VGA_GC_INDEX, i);
                *regs = inb(VGA_GC_DATA);
                regs++;
        }
        for (i = 0; i < VGA_NUM_AC_REGS; i++)
        {
                (void)inb(VGA_INSTAT_READ);
                outb(VGA_AC_INDEX, i);
                *regs = inb(VGA_AC_READ);
                regs++;
        }
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, 0x20);
}

void write_regs(uint8_t *regs)
{
        uint32_t i;

        outb(VGA_MISC_WRITE, *regs);
        regs++;
        for (i = 0; i < VGA_NUM_SEQ_REGS; i++)
        {
                outb(VGA_SEQ_INDEX, i);
                outb(VGA_SEQ_DATA, *regs);
                regs++;
        }
        outb(VGA_CRTC_INDEX, 0x03);
        outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
        outb(VGA_CRTC_INDEX, 0x11);
        outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
        regs[0x03] |= 0x80;
        regs[0x11] &= ~0x80;
        for (i = 0; i < VGA_NUM_CRTC_REGS; i++)
        {
                outb(VGA_CRTC_INDEX, i);
                outb(VGA_CRTC_DATA, *regs);
                regs++;
        }
        for (i = 0; i < VGA_NUM_GC_REGS; i++)
        {
                outb(VGA_GC_INDEX, i);
                outb(VGA_GC_DATA, *regs);
                regs++;
        }
        for (i = 0; i < VGA_NUM_AC_REGS; i++)
        {
                (void)inb(VGA_INSTAT_READ);
                outb(VGA_AC_INDEX, i);
                outb(VGA_AC_WRITE, *regs);
                regs++;
        }
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, 0x20);
}

static void (*g_write_pixel)(uint32_t x, uint32_t y, uint32_t c);
static uint32_t g_wd, g_ht;

void VGASetPal(char *pal, char first, short num)
{
        if ((num + first) > 256)
                num = 256 - first;
        if (!num)
                return;
        num *= 3;
        outb(VGA__DAC_ADDR_W, first);
        while (num--)
        {
                outb(VGA__DAC_DATA, (*pal));
                pal++;
        }
}

void set_color_palette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
        outb(0x3C8, index);
        outb(0x3C9, red);
        outb(0x3C9, green);
        outb(0x3C9, blue);
}

void mode(void)
{
        write_regs(g_320x200x256);

        char PAL256[256 * 3];
        size_t x = 0;
        for (int r = 0; r < 4; ++r)
        {
                for (int g = 0; g < 4; ++g)
                {
                        for (int b = 0; b < 4; ++b)
                        {
                                PAL256[x++] = (r * 85);
                                PAL256[x++] = (g * 85);
                                PAL256[x++] = (b * 85);
                        }
                }
        }

        for (int level = 0; level < 32; ++level)
        {
                PAL256[x++] = level * 2;
                PAL256[x++] = level * 2;
                PAL256[x++] = level * 2;
        }

        VGASetPal(PAL256, 0, 256);
        TTY_WIDTH = 40;
        TTY_HEIGHT = 25;
}

#endif