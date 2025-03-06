#include <stdint.h>
#include <stdbool.h>

#define DEBUG
#define VERBOSE
#define MAX_ADDR (1<<25)
#define _OSA86

#pragma pack(1)

#define NULL (void*)(0)
#define cli() asm ("cli")
#define sti() asm ("sti")
#define __VER__ "0.4"

uint8_t Drive_Letter = 'A';

void osa86();
void clearScreen(uint8_t c);

uint8_t hour, minute, second;
uint8_t day, month, year;

void init()
{
        osa86();
}

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

void mlmon(char * filename)
{
        uint8_t mem[8192];
        memset(mem,0,sizeof(mem));
        if (Exists(filename)!=0)
        {
                FILE fp = fgetf(filename,current_path_idx);
                fseek(&fp,0,SEEK_END);
                int len = ftell(&fp);
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
                clearScreen(TTY_COL);
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
        clearScreen(TTY_COL);
}

#define CMDM_LEN 128
#define MARGS 32

int
parse(b,cmd)
        char * b,cmd[MARGS][CMDM_LEN];
{       int N,i;
        for (N=0,i=0;*b&&N<MARGS&&i<CMDM_LEN;cmd[N++][i]=0,++b,i=0) {
                while (*b == ' ') b++;
                if(*b=='"'){for(++b;*b!='"'&&*b;++b,++i)
                  cmd[N][i]=(*b=='\\')?((*(++b)=='n')?'\n':*b):*b;}
                else for(;*b&&*b!=' ';cmd[N][i++]=*(b++)); }
        return N;
}

void system(char* sys)
{
        char cmd[MARGS][CMDM_LEN];
        memset(cmd,0,sizeof(cmd));
        int argc=parse(sys,(char (*)[128])&cmd);
        if (strncmp(cmd[0],"./",2)==0) {
                char* path = cmd[0]+2;
                while(path[0] == '/')path++;
                int res = ExecuteF(path);
                if (res!=0)
                {
                        r=res;
                }
        } else if (strcmp(cmd[0], "info") == 0) { 
                printf("OSA86 VERSION %s\n(C) JOSHUA F. 2024-2025\n",__VER__);
                uint32_t remainingSpace = remaining_heap_space();
                printf("Heap Size Is %d Bytes\n",remainingSpace);
                printf("File Descriptor Size Is %d Bytes\n",sizeof(FileDescriptor));
                rtc_get_time(&hour, &minute, &second);
                rtc_get_date(&day, &month, &year);
                printf("Time: %d:%d:%d, ", hour, minute, second);
                printf("Date: %d/%d/%d\n", day, month, year);
        } else if (strcmp(cmd[0], "&") == 0 && argc>1) {
                int i;
                for (i=1;i<argc;++i)
                {
                        ExecuteF(cmd[i]);
                }
                ListSchedule();
        } else if (strcmp(cmd[0], "cls") == 0) { 
                clearScreen(TTY_COL);
        } else if (strcmp(cmd[0], "?") == 0) { 
                printf("%d\n",r);
        } else if (strcmp(cmd[0], "ls") == 0) {
                ListF();
        } else if (strcmp(cmd[0], "cd") == 0 && argc==2) {
                Cd(cmd[1]);
        } else if (strcmp(cmd[0], "shutdown") == 0) {
                active = false;
                cli();
        } else if (strcmp(cmd[0], "expr") == 0) {
                if (argc == 2) BrainFuck(cmd[1]);
        } else if (strcmp(cmd[0], "pKill") == 0 && argc == 2) {
                int i = atoi(cmd[1]);
                PKill(i);
        } else if (strcmp(cmd[0], "view") == 0 && argc == 2) {
                FILE *fp = fopen(cmd[1],"r");
                char *data = wfread(fp);
                printf("%s",data);
                free(data);
                fclose(fp);
        } else if (strcmp(cmd[0], "write") == 0 && argc == 3) {
                WriteF(cmd[2],cmd[1],strlen(cmd[1]));
        } else if (strcmp(cmd[0], "create") == 0 && argc == 2) {
                CreateF(cmd[1]);
        } else if (strcmp(cmd[0], "rm") == 0 && argc == 2) {
                DeleteF(cmd[1]);
        } else if (strcmp(cmd[0], "tasks") == 0) {
                ListSchedule();
        } else if (strcmp(cmd[0], "mlmon") == 0 && argc == 2) {
                mlmon(cmd[1]);
        } else if (strcmp(cmd[0], "time") == 0) {
                rtc_get_time(&hour, &minute, &second);
                rtc_get_date(&day, &month, &year);
                printf("Time: %d:%d:%d, ", hour, minute, second);
                printf("Date: %d/%d/%d\n", day, month, year);
        } else if (cmd[0][0]) {
                _ExecuteF(cmd[0],-1); // Execute file in root
        }
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
        system("info");
        putc('\n');

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
        elf(outf,0);
        fclose(outf);

        char kbbuff[128];
        while (active)
        {
                /* Yes */
                sti();

                /* Shell Mainloop */
                memset(kbbuff,0,sizeof(kbbuff));
                printf("%c:/%s%c%s> ", Drive_Letter, ActiveDirParen(), ActiveDirParen()[0]==0 ? '\0' : '/', ActiveDir());
                gets(kbbuff, 128);
                system(kbbuff);
        }

        cli();
        mode(0x2);
        clearScreen(TTY_COL);
        printf("\xff[20x11y]It is now safe to turn of your computer\n");
        while(1);
}
