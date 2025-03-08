#include <stdint.h>
#include <stdbool.h>

#define DEBUG
#define VERBOSE
#define _OSA86

typedef uint8_t PALETTE16[16][3];

//PALETTE16 PalExample =
//{
//        /*black*/  {23,24,33},
//        /*blue*/   {1,100,188},
//        /*green*/  {80,161,79},
//        /*cyan*/   {9,151,179},
//        /*red*/    {229,86,73},
//        /*magent*/ {166,38,164},
//        /*yellow*/ {192,131,1},
//        /*white*/  {250,250,250},
//        /*black*/  {23,24,33},
//        /*blue*/   {1,100,188},
//        /*green*/  {80,161,79},
//        /*cyan*/   {9,151,179},
//        /*red*/    {229,86,73},
//        /*magent*/ {166,38,164},
//        /*yellow*/ {192,131,1},
//        /*white*/  {250,250,250},
//};

#pragma pack(1)

#define NULL (void*)(0)
#define cli() asm ("cli")
#define sti() asm ("sti")
#define __VER__ "0.4.1"

void osa86();
void clearScreen(uint8_t c);
void refresh();
void initputs(char *);

int  MAX_ADDR;

void init(int memSize)
{
        if (memSize == 0)
        {
                uint16_t *b = (uint16_t *)0xB8000;
                for (int i = 0; i < 80*25; ++i)
                {
                        b[i] = 0x1720;
                }
                initputs("sorry, but you need at least two megabytes of ram to use osa86");
                while(1);
        }
        MAX_ADDR=(memSize*1024)+1024*1024;
        osa86();
}

void initputs(char *s)
{
        char *buff = (char *)((10*160)+(0xB8000)+16);
        while (*s)
        {
                *buff = *(s++);
                *(buff+1) = 0x17;
                buff += 2;
        }
}

uint8_t Drive_Letter = 'A';
uint8_t hour, minute, second;
uint8_t day, month, year;

void send_eoi(uint8_t irq);

static int r=0;
bool active = true;

#include "io.h"
#include "string.h"
#include "terminal.h"
#include "pic.h"
#include "idt.h"
#include "gdt.h"
#include "malloc.h"
#include "pit.h"
#include "sound.h"
#include "osafs2.h"
#include "video.h"
#include "user.h"
#include "program.h"
#include "schedule.h"
#include "rtc.h"
#include "elf.h"
#include "shell.h"
#include "mlmon.h"

void osa86()
{
        cli();
        mode(0x02);

        init_tty();
        init_heap();
        init_gdt();
        init_idt();
        init_pic();
        init_scheduler();
        init_fs();
        init_pit(128);
        FILE *test0 = fopen("test.txt","w");
        FILE *test1 = fopen("test2.txt","w");
        char msg[] = "Hello, World!";
        fwrite(msg,1,sizeof(msg),test0);
        fwrite(msg,1,sizeof(msg),test1);
        fclose(test0);
        fclose(test1);

        char prog[] = {
                        'O','S','A','X',0xAA,0x0D,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
                        0xBB,0,0,0,0,0xB8,1,0,0,0,0x50,0x53,0xcd,0x80,0xeb,0xfe
                      };
        WriteF("test",prog,sizeof(prog));
        char out[] = {79,83,65,88,-86,13,0,0,0,1,0,0,0,-24,16,0,0,0,-69,0,0,0,0,-72,1,0,0,0,80,83,-51,-128,-21,-2,-24,44,0,0,0,-61,-21,-2,-95,126,0,0,0,-93,122,0,0,0,-95,-126,0,0,0,-93,126,0,0,0,-95,122,0,0,0,-117,29,126,0,0,0,1,-40,-93,-126,0,0,0,-61,-21,-2,-72,1,0,0,0,-69,1,0,0,0,57,-40,15,-108,-64,117,19,-95,122,0,0,0,-69,1,0,0,0,1,-40,-93,122,0,0,0,-21,-36,-61,-21,-2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
        WriteF("out",out,sizeof(out));

        FILE *outf = fopen("out","r");
        fclose(outf);

        AppendTask("shell",shell);
        AppendTask("tty",refresh);
        system("info");
        while (active)
        {
                /* Yes */
                sti();
        }

        cli();
        mode(0x2);
        clearScreen(TTY_COL);
        printf("\xff[20x11y]It is now safe to turn of your computer\n");
        while(1);
}
