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
void initputs(char *,int,int);

int MAX_ADDR;
int ram_size(int);

void init(int memSize)
{
        (void)memSize;
        uint16_t *b = (uint16_t *)0xB8000;
        for (int i = 0; i < 80*25; ++i)
        {
                b[i] = 0x1720;
        }
        initputs("Starting OSA86",(80-15)/2,10);
        initputs("Detecting Memory",(80-17)/2,11);
        MAX_ADDR=ram_size(0);
        if (MAX_ADDR <= 1024*1024)
        {
                initputs("Sorry, but you need at least two megabytes of ram to use osa86",8,10);
                initputs("Please get more ram",(80-20)/2,11);
                while(1);
        }
        osa86();
}

void initputs(char *s, int x, int y)
{
        char *buff = (char *)((y*160)+(0xB8000)+(x*2));
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
#include "schedule.h"
#include "rtc.h"
#include "elf.h"
#include "shell.h"
#include "mlmon.h"

#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_DIRECTORY_ENTRIES 1024

uint32_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

uint8_t bitmap[PAGE_TABLE_ENTRIES/8];

void* alloc_page()
{
        for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++)
        {
                if (!(bitmap[i / 8] & (1 << (i % 8))))
                {
                        bitmap[i / 8] |= (1 << (i % 8));
                        return (void*)(i * PAGE_SIZE); 
                }
        }
        return NULL;
}

int ram_size(int off)
{
        uint32_t *addr = (uint32_t *)(0x100000+off);
        int size = 0;
        while(1)
        {
                *addr = 0x69696969;
                if (*addr == 0x69696969)
                {
                        addr += 1;
                        size += 4;
                        continue;
                }
                return size + 1024*1024;
        }
}

void setup_paging()
{
        initputs("Setting up pages",(80-17)/2,11);
        uint32_t num_page_tables = ((MAX_ADDR + 0x3FFFFF) / 0x400000); // Round up
        uint32_t *page_tables[num_page_tables];
        for (uint32_t i = 0; i < num_page_tables; i++)
        {
                page_tables[i] = (uint32_t*)alloc_page(); // Allocate a 4 KB page for each page table
                for (uint32_t j = 0; j < PAGE_TABLE_ENTRIES; j++)
                {
                        uint32_t physical_addr = (i * 1024 + j) * PAGE_SIZE;
                        page_tables[i][j] = physical_addr | 0x03; // Present + Read/Write
                }
        }

        for (uint32_t i = 0; i < PAGE_DIRECTORY_ENTRIES; i++)
        {
                page_directory[i] = 0x00000002; // Not present
        }

        for (uint32_t i = 0; i < num_page_tables; i++)
        {
                page_directory[i] = ((uint32_t)page_tables[i]) | 0x03; // Present + Read/Write
        }

        asm volatile("mov %0, %%cr3" : : "r"((uint32_t)page_directory));
        uint32_t cr0;
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000; // Set PG bit
        asm volatile("mov %0, %%cr0" : : "r"(cr0));
        ((uint8_t*)0xB8000)[0] = 'P';
        ((uint8_t*)0xB8000)[1] = 0x1F;
}

void osa86()
{
        cli();
        memset(bitmap, 0, BITMAP_SIZE); // Mark all pages as free
        setup_paging();

        init_tty();
        init_heap();
        init_gdt();
        init_idt();
        init_pic();
        init_scheduler();
        init_fs();
        init_pit(512);
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

        clearScreen(TTY_COL);
        IterateSchedule(i)
        {
                printf("ending %s\n", current->name);
                PKill(i);
        }

        mode(0x2);
        clearScreen(TTY_COL);
        printf("\xff[20x11y]It is now safe to turn of your computer\n");
        flush();
        while(1);
}
