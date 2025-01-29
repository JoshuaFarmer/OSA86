#include <stdint.h>
#include <stdbool.h>

#define DEBUG
#define VERBOSE

#pragma pack(1)

#define NULL (void*)(0)
#define cli() asm ("cli")
#define sti() asm ("sti")
#define __VER__ "0.2"

uint8_t Drive_Letter = 'A';

void osa86();
void clearScreen(uint8_t c);

void init() {
        osa86();
}

void send_eoi(uint8_t irq);

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
#include "gui.h"
#include "program.h"
#include "schedule.h"

void mlmon(char * filename)
{
        uint8_t mem[8192];
        memset(mem,0,sizeof(mem));
        if (Exists(filename)!=0)
        {
                FILE fp = fgetf(filename,current_path_idx);
                fseek(fp,0,SEEK_END);
                int len = ftell(fp);
                ReadF(filename, mem, len);
        }

        char input = 0;
        int x=0;
        int y=0;
        char buff[3];
        int page=0;
        while (input != 'q')
        {
                int pos=x+(y*16);
                clearScreen(0x1F);
                printf("Page: "); PrintByte(page); putc('\n');
                for (int i = 1; i <= 256; ++i)
                {
                        PrintByte(mem[(i-1)+(256*page)]); putc(' ');
                        if ((i % 16) == 0)
                        {
                                putc('\n');
                        }
                }
                update_cursor(x*3,y+1);
                switch (input)
                {
                        case 's':
                                if (Exists(filename)==0)
                                        CreateF(filename);
                                WriteF(filename,mem,sizeof(mem));
                                break;
                        case 'H':
                                if (page)
                                        --page;
                                break;
                        case 'L':
                                if (page < 31)
                                        ++page;
                                break;
                        case 'h':
                                if (x)
                                        --x;
                                break;
                        case 'l':
                                if (x < 15)
                                        ++x;
                                break;
                        case 'j':
                                if (y < 15)
                                        ++y;
                                break;
                        case 'k':
                                if (y)
                                        --y;
                                break;
                        case '\n':
                                getsf(buff,3,x*3,y+1,'\n');
                                mem[pos+(256*page)]=strhex(buff);
                                if (x < 15)
                                        ++x;
                                else if (y < 15)
                                {
                                        ++y;
                                        x=0;
                                }
                                else
                                {
                                        y=0;
                                        x=0;
                                        ++page;
                                }
                                break;
                }
                update_cursor(x*3,y+1);
                input = getch();
        }
        clearScreen(0x2);
}

void system(char* _syscmd) {
        char* syscmd  = strtok(_syscmd, " \0");
        char* syscmd1 = strtok(NULL, ";\0");
        char* syscmd2 = strtok(NULL, "\0");

        static int r=0;
        if (strncmp(syscmd, "./", 2) == 0) {
                char* path = syscmd+2;
                while(path[0] == '/')path++;
                int res = ExecuteF(path);
                r=res;
        } else if (strcmp(syscmd, "info") == 0) { 
                puts("OSA86 VERSION "); puts(__VER__); puts("\n(C) JOSHUA F. 2024-2025\n");
        } else if (strcmp(syscmd, "&") == 0) { 
                if (syscmd1 && syscmd2)
                {
                        Schedule(syscmd1);
                        Schedule(syscmd2);
                }
        } else if (strcmp(syscmd, "cls") == 0) { 
                clearScreen(termCol);
        } else if (strcmp(syscmd, "?") == 0) { 
                put_int(r);
                putc('\n');
        } else if (strcmp(syscmd, "ls") == 0) {
                ListF();
        } else if (strcmp(syscmd, "cd") == 0) {
                if (syscmd1)
                {
                        Cd(syscmd1);
                }
        } else if (strcmp(syscmd, "shutdown") == 0) {
                active = false;
        } else if (strcmp(syscmd, "expr") == 0) {
                BrainFuck(syscmd1);
        } else if (strcmp(syscmd, "view") == 0) {
                FILE fp = fgetf(syscmd1,current_path_idx);
                fseek(fp,0,SEEK_END);
                int len = ftell(fp);
                char* data = malloc(len);
                char* path = syscmd1;
                
                ReadF(path,data,len);
                putsn((const char*)data, len);
                free(data);
                data=path=NULL;
        } else if (strcmp(syscmd, "write") == 0) {
                if (syscmd1 && syscmd2)
                {
                        WriteF(syscmd2,syscmd1,strlen(syscmd1));
                }
        } else if (strcmp(syscmd, "create") == 0) {
                if (syscmd1)
                {
                        CreateF(syscmd1);
                }
        } else if (strcmp(syscmd, "rm") == 0) {
                if (syscmd1)
                {
                        DeleteF(syscmd1);
                }
        } else if (strcmp(syscmd, "mlmon") == 0) {
                if (syscmd1)
                {
                        mlmon(syscmd1);
                }
        } else {
                if (syscmd[1] == ':') {
                        //switch_drive(syscmd[0] - 'A');
                }
                else if (syscmd[0]) { puts(syscmd); puts(", What?\n"); }
        }
}

void mode(int mod) {
        int x=0;
        switch (mod)
        {
                case 0x00:
                        write_regs(g_320x200x256);
                        break;
                case 0x01:
                        x = set_text_mode(1);
                        break;
                case 0x02:
                        x = set_text_mode(0);
                        break;
                case 0x03:
                        x = set_text_mode(2);
                        break;
                case 0x04:
                        write_regs(g_640x480x2);
                        break;
        }

        TTY_WIDTH = (x >> 16) & 0xFFFF;
        TTY_HEIGHT = x & 0xFFFF;

        if (x == 0 && mod != 0) {
                mode(0x1);
        }
}

void yield(int steps)
{
        for(int i = 0; i < steps;++i);
}

void osa86() {
        cli();
        clearScreen(termCol);
        mode(0x02);
        system("info");

        putc('\n');
        init_heap();
        init_gdt();
        init_idt();
        init_pic();
        init_pit(100);
        putc('\n');

        InitRamFS();
        CreateF("test.txt");
        CreateF("test.tx");
        CreateF("test");
        WriteF("test.txt","Hellorld!\n",11);
        WriteF("test.tx","Hellorld!2\n",12);

        char prog[] = { 0x4F,0x53,0x41,0x58,
                        0xAA,0x0D,0x00,0x00,
                        0x00,0x01,0x00,0x00,
                        0x00,0x31,0xC0,0xC3 };
        WriteF("test",prog,sizeof(prog));

        char* kbbuff = malloc(128);

        uint32_t remainingSpace = remaining_heap_space();
        printf("Heap Size Is %d Bytes\n",remainingSpace);
        printf("File Descriptor Size Is %d Bytes\n",sizeof(FileDescriptor));

        // E.g.
        //SysCall x;
        //x.code=0x0;
        //x.a='_';
        //Int80(&x);

        while (active)
        {
                sti();
                yield(100);
                cli();
                printf("%c: %s/%s> ", Drive_Letter, ActiveDirParen(), ActiveDir());
                gets(kbbuff, 128);
                system(kbbuff);
        }

        free(kbbuff);
        clearScreen(termCol);
        mode(0x2);
        puts_at("It is now safe to turn of your computer\n", 20, 11);
}
