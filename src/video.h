#ifndef VIDEO_H
#define VIDEO_H

#include "io.h"
#include "font.h"

#define VGA_AC_INDEX                0x3C0
#define VGA_AC_WRITE                0x3C0
#define VGA_AC_READ                 0x3C1
#define VGA_MISC_WRITE              0x3C2
#define VGA_SEQ_INDEX               0x3C4
#define VGA_SEQ_DATA                0x3C5
#define VGA_DAC_READ_INDEX          0x3C7
#define VGA_DAC_WRITE_INDEX         0x3C8
#define VGA_DAC_DATA                0x3C9
#define VGA_MISC_READ               0x3CC
#define VGA_GC_INDEX                0x3CE
#define VGA_GC_DATA                 0x3CF
#define VGA_CRTC_INDEX              0x3D4
#define VGA_CRTC_DATA               0x3D5
#define VGA_INSTAT_READ             0x3DA
#define VGA_NUM_SEQ_REGS            0x005
#define VGA_NUM_CRTC_REGS           0x019
#define VGA_NUM_GC_REGS             0x009
#define VGA_NUM_AC_REGS             0x015
#define VGA_NUM_REGS                (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
#define VGA__DAC_ADDR_W             0x3c8
#define VGA__DAC_DATA               0x3c9

uint8_t g_40x25_text[] =
{
        0x67,
        0x03, 0x08, 0x03, 0x00, 0x02,
        0x2D, 0x27, 0x28, 0x90, 0x2B, 0xA0, 0xBF, 0x1F,
        0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0xA0,
        0x9C, 0x8E, 0x8F, 0x14, 0x1F, 0x96, 0xB9, 0xA3,
        0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
        0xFF,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x0C, 0x00, 0x0F, 0x08, 0x00,
};

uint8_t g_80x25_text[] =
{
        0x67,
        0x03, 0x00, 0x03, 0x00, 0x02,
        0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
        0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
        0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
        0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
        0xFF,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x0C, 0x00, 0x0F, 0x08, 0x00
};

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
        0x41, 0x00, 0x0F, 0x00, 0x00
};

static void dump(uint8_t *regs, uint32_t count)
{
        uint32_t i;

        i = 0;
        for(; count != 0; count--)
        {
                i++;
                if(i >= 8)
                {
                        i = 0;
                }
                regs++;
        }
}

static void set_plane(uint32_t p)
{
        uint8_t pmask;

        p &= 3;
        pmask = 1 << p;
        outb(VGA_GC_INDEX, 4);
        outb(VGA_GC_DATA, p);
        outb(VGA_SEQ_INDEX, 2);
        outb(VGA_SEQ_DATA, pmask);
}

static uint32_t get_fb_seg(void)
{
        uint32_t seg;

        outb(VGA_GC_INDEX, 6);
        seg = inb(VGA_GC_DATA);
        seg >>= 2;
        seg &= 3;
        switch(seg)
        {
        case 0:
        case 1:
                seg = 0xA000;
                break;
        case 2:
                seg = 0xB000;
                break;
        case 3:
                seg = 0xB800;
                break;
        }
        return seg;
}

static void vmemwr(uint32_t dst_off, uint8_t *src, uint32_t count)
{
        _vmemwr(get_fb_seg(), dst_off, src, count);
}

void dump_regs(uint8_t *regs)
{
        regs++;
        dump(regs, VGA_NUM_SEQ_REGS);
        regs += VGA_NUM_SEQ_REGS;
        dump(regs, VGA_NUM_CRTC_REGS);
        regs += VGA_NUM_CRTC_REGS;
        dump(regs, VGA_NUM_GC_REGS);
        regs += VGA_NUM_GC_REGS;
        dump(regs, VGA_NUM_AC_REGS);
        regs += VGA_NUM_AC_REGS;
}

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
        for(i = 0; i < VGA_NUM_AC_REGS; i++)
        {
                (void)inb(VGA_INSTAT_READ);
                outb(VGA_AC_INDEX, i);
                outb(VGA_AC_WRITE, *regs);
                regs++;
        }
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, 0x20);
}


static void write_font(uint8_t *buf, uint32_t font_height)
{
        uint8_t seq2, seq4, gc4, gc5, gc6;
        uint32_t i;

        outb(VGA_SEQ_INDEX, 2);
        seq2 = inb(VGA_SEQ_DATA);

        outb(VGA_SEQ_INDEX, 4);
        seq4 = inb(VGA_SEQ_DATA);
        outb(VGA_SEQ_DATA, seq4 | 0x04);

        outb(VGA_GC_INDEX, 4);
        gc4 = inb(VGA_GC_DATA);

        outb(VGA_GC_INDEX, 5);
        gc5 = inb(VGA_GC_DATA);
        outb(VGA_GC_DATA, gc5 & ~0x10);

        outb(VGA_GC_INDEX, 6);
        gc6 = inb(VGA_GC_DATA);
        outb(VGA_GC_DATA, gc6 & ~0x02);
        set_plane(2);
        for (i = 0; i < 256; i++)
        {
                vmemwr(16384u * 0 + i * 32, buf, font_height);
                buf += font_height;
        }
        outb(VGA_SEQ_INDEX, 2);
        outb(VGA_SEQ_DATA, seq2);
        outb(VGA_SEQ_INDEX, 4);
        outb(VGA_SEQ_DATA, seq4);
        outb(VGA_GC_INDEX, 4);
        outb(VGA_GC_DATA, gc4);
        outb(VGA_GC_INDEX, 5);
        outb(VGA_GC_DATA, gc5);
        outb(VGA_GC_INDEX, 6);
        outb(VGA_GC_DATA, gc6);
}

static void (*g_write_pixel)(uint32_t x, uint32_t y, uint32_t c);
static uint32_t g_wd, g_ht;

void dump_state(void)
{
        uint8_t state[VGA_NUM_REGS];

        read_regs(state);
        dump_regs(state);
}

uint32_t set_text_mode(int hi_res)
{
        uint16_t rows, cols, ht, i;

        if (hi_res == 1||hi_res == 2)
        {
                write_regs(g_40x25_text);
                cols = 40;
                rows = 25;
                ht = 16;
        }
        else 
        {
                write_regs(g_80x25_text);
                cols = 80;
                rows = 25;
                ht = 16;
        }
        /* set font */
        if(ht >= 16)
                write_font(g_8x16_font, 16);
        /* tell the BIOS what we've done, so BIOS text output works OK */
        pokew(0x40, 0x4A, cols);
        pokew(0x40, 0x4C, cols * rows * 2);
        pokew(0x40, 0x50, 0);
        pokeb(0x40, 0x60, ht - 1);
        pokeb(0x40, 0x61, ht - 2);
        pokeb(0x40, 0x84, rows - 1);
        pokeb(0x40, 0x85, ht);
        /* set white-on-black attributes for all text */
        for(i = 0; i < cols * rows; i++)
                pokeb(0xB800, i * 2 + 1, 7);
        return (cols << 16) + rows;
}

void VGASetPal(char*pal,char first,short num)
{
        if((num+first)>256)num=256-first;
        if(!num)return;
        num*=3;
        outb(VGA__DAC_ADDR_W,first);
        while(num--)
        {
                outb(VGA__DAC_DATA,(*pal));
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

void mode(int mod)
{
        int x=0;
        switch (mod)
        {
                case 0x00:
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

                        VGASetPal(PAL256, 0, 256);
                        break;
                }
                case 0x02:
                        x = set_text_mode(0);
                        break;
                case 0x03:
                        x = set_text_mode(2);
                        break;
        }

        TTY_WIDTH = (x >> 16) & 0xFFFF;
        TTY_HEIGHT = x & 0xFFFF;
}

#endif