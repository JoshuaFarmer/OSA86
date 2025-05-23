/* basically emojis, thanks to my brother @haha_yes123 (YT) for help in designing */
#ifndef GOOBERCONS_H
#define GOOBERCONS_H

typedef struct GooberCon
{
        uint8_t bitmap[8];
        char code[8]; /* name but short, TYPE.NAM (e.g. FAC__SAD) */

} GooberCon;

#define GOOBER_CON_WIDTH 8
#define GOOBER_CON_HEIGHT 8
#define GOOBER_COUNT 31

const GooberCon gooberCons[GOOBER_COUNT] =
    {
        [12] = {
            .bitmap =
                {
                    0b11111111,
                    0b10000001,
                    0b10100011,
                    0b10000001,
                    0b10011101,
                    0b10100011,
                    0b10000001,
                    0b11111111,
                },
            .code = "FAC_SAD2"},

        [13] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100011,
                   0b10000001,
                   0b10100011,
                   0b10011101,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_HA2"},
        
        [14] = {
            .bitmap =
                {
                    0b11111111,
                    0b10000001,
                    0b10100011,
                    0b10000001,
                    0b10011101,
                    0b10011101,
                    0b10000001,
                    0b11111111,
                },
            .code = "FAC_OOO2"},
        [10] = {.bitmap = {
                   0b11111111,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               },
               .code = "SYM_W00"},
        [11] = {.bitmap = {
                   0b11111111,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
               },
               .code = "SYM_W01"},
        [16] = {.bitmap = {
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b11111111,
               },
               .code = "SYM_W02"},
        [17] = {.bitmap = {
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b11111111,
               },
               .code = "SYM_W03"},
        [15] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100011,
                   0b10000001,
                   0b10011101,
                   0b10000001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_MEH2"},
        [0] = {
            .bitmap =
                {
                    0b11111111,
                    0b10000001,
                    0b10100101,
                    0b10000001,
                    0b10011001,
                    0b10100101,
                    0b10000001,
                    0b11111111,
                },
            .code = "FAC_SAD1"},

        [1] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100101,
                   0b10000001,
                   0b10100101,
                   0b10011001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_HAP1"},
        
        [6] = {
            .bitmap =
                {
                    0b11111111,
                    0b10000001,
                    0b10100101,
                    0b10000001,
                    0b10011001,
                    0b10011001,
                    0b10000001,
                    0b11111111,
                },
            .code = "FAC_OOO1"},

        [7] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100101,
                   0b10000001,
                   0b10011001,
                   0b10000001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_MEH1"},

        [8] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10111101,
                   0b10111101,
                   0b10100001,
                   0b10100001,
                   0b10000001,
                   0b11111111,
               },
               .code = "SYM_FLAG"},

        [9] = {.bitmap = {
                   0b11111111,
                   0b10011001,
                   0b10111101,
                   0b11100111,
                   0b10111101,
                   0b10111101,
                   0b10111101,
                   0b11111111,
               },
               .code = "SYM_HOUS"},

        [2] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100101,
                   0b10100101,
                   0b10011001,
                   0b10100101,
                   0b10000001,
                   0b11111111,
               },
               .code = "SYM_EXIT"},
        [3] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10000101,
                   0b10001001,
                   0b10101001,
                   0b10010001,
                   0b10000001,
                   0b11111111,
               },
               .code = "SYM_CHCK"},
        [4] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10011101,
                   0b10000101,
                   0b10010101,
                   0b10100001,
                   0b10000001,
                   0b11111111,
               },
               .code = "SYM_ARR1"},
        [5] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100001,
                   0b10010101,
                   0b10000101,
                   0b10011101,
                   0b10000001,
                   0b11111111,
               },
               .code = "SYM_ARR2"},
        [18] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100101,
                   0b10000001,
                   0b10010101,
                   0b10101001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_TLID"},
        [19] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10000101,
                   0b10100001,
                   0b10000101,
                   0b10111101,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_GOOB"},
        [20] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100101,
                   0b10000001,
                   0b10000001,
                   0b10000001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_NUL1"},
        [21] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10100011,
                   0b10000001,
                   0b10000001,
                   0b10000001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_NUL2"},
        [22] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10000001,
                   0b11000011,
                   0b10011001,
                   0b10000001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_GOO2"}, /* goober 2 */
        [23] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b11111111,
                   0b11100111,
                   0b10000001,
                   0b10011001,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_SHAD"}, /* shades */
        [25] = {.bitmap = {
                   0b11111111,
                   0b10101011,
                   0b11010101,
                   0b10101011,
                   0b11010101,
                   0b10101011,
                   0b11010101,
                   0b11111111,
               },
               .code = "CHECKER"}, /* checkboard pattern */
        [26] = {.bitmap = {
                   0b11111111,
                   0b10111101,
                   0b10000001,
                   0b11000011,
                   0b10011001,
                   0b10111101,
                   0b10000001,
                   0b11111111,
               },
               .code = "FAC_DOG0"},
        [27] = {.bitmap = {
                   0b11111111,
                   0b11000001,
                   0b10100001,
                   0b10010001,
                   0b10001001,
                   0b10000101,
                   0b10000011,
                   0b11111111,
               },
               .code = "SYM_LINE"},
        [28] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b10011101,
                   0b10110001,
                   0b10111101,
                   0b10010101,
                   0b10000001,
                   0b11111111,
               },
               .code = "SYM_SUS0"},
        [29] = {.bitmap = {
                   0b11111111,
                   0b10000001,
                   0b11000011,
                   0b10011001,
                   0b10100101,
                   0b10100101,
                   0b10011001,
                   0b11111111,
               },
               .code = "FAC_OOO3"},
        [24] = {.bitmap = {
                   0b11111111,
                   0b10111101,
                   0b11000011,
                   0b11010111,
                   0b11000011,
                   0b10111101,
                   0b10001001,
                   0b10011101,
               },
               .code = "GUY_BOBT"}, /* BOB . TOP */
        [30] = {.bitmap = {
                   0b10101011,
                   0b11001011,
                   0b10001001,
                   0b10001001,
                   0b10010101,
                   0b10110101,
                   0b10100101,
                   0b11111111,
               },
               .code = "GUY_BOBB"}, /* BOB . BOTTOM */
};

void drawgoobercon(int goober, int x, int y, int bg, int fg)
{
        bool flipped = (goober >> 7) & 1; /* bit 7 is used for direction */
        bool invert  = (goober >> 6) & 1; /* bit 6 is used for inverting colour */
        goober &= 31;
        char *buff = (char *)0xA0000;
        for (int row = 0; row < GOOBER_CON_HEIGHT; ++row)
        {
                int byte = gooberCons[goober].bitmap[row];
                for (int bit = 0; bit < GOOBER_CON_WIDTH; ++bit)
                {
                        int index = (y + row) * VGA_WIDTH + x + ((flipped) ? bit : ((GOOBER_CON_WIDTH-1) - (bit)));
                        buff[index] = (invert == ((byte >> bit) & 1)) ? fg : (bg != 255) ? bg
                                                                             : buff[index];
                }
        }
}

int gooberFromName(char *name)
{
        for (int i = 0; i < 31; ++i)
        {
                if (memcmp(name,gooberCons[i].code,8) == 0)
                {
                        return i;
                }
        }

        return -1;
}

#endif