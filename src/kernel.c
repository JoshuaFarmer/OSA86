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

void mlmon(char * filename)
{
        if (Exists(filename)==0)
        {
                CreateF(filename);
        }

        FILE fp = fgetf(filename,current_path_idx);
        fseek(fp,0,SEEK_END);
        int len = ftell(fp);

        uint8_t mem[8192];
        memset(mem,0,sizeof(mem));
        ReadF(filename, mem, len);
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

void osa86() {
        cli();
        clearScreen(termCol);
        mode(0x02);
        system("info");

        putc('\n');
        init_heap();
        init_gdt();
        init_pit(100);
        init_pic();
        init_idt();
        putc('\n');

        Beep(1000);

        //super_block = malloc(sizeof(Super_Block_t));
        //initialize_super_block(super_block);
        //create("link.lnk", false);
        //create("ExampleDir", true);
        //create("ExampleDir/hello.txt", false);
        //create("example.prg", false);
        //create("example2.exe", false);
        InitRamFS();
        CreateF("test.txt");
        CreateF("test.exe");
        WriteF("test.txt","Hellorld!\n",11);

        FILE * fp = fopen("test.exe","wb");
        fputc(0xb8,fp);
        fputc(0x20,fp);
        fputc(0x00,fp);
        fputc(0x00,fp);
        fputc(0x00,fp);
        fputc(0xC3,fp);
        fclose(fp);
/*
        for (int i = 0; i < 16; ++i) {
                char num[32];
                itoa(i, num, 16);
                char* str = strcat(num, ".txt");
                create(str, false);
                free(str);
        }

        write_file("link.lnk", "ExampleDir/hello.txt\0", 22);
        write_file("ExampleDir/hello.txt", "Hello, World!\n\0", 16);

        char* example_expr = ">++++++[<++++++++++>-]<+++++>+[<[>>+>+<<<-]>>>[<<<+>>>-]<+>>+++++++++[<++++++++++>-]<>+[<+>-]>+<<<[>>+<<-]>>[>-]>[><<<<+>[-]>>->]<+<<[>-[>-]>[><<<<+>[-]+>>->]<+<<-]>[-]>-<<<[<<[>>>+>+<<<<-]>>>>[<<<<+>>>>-]<.[-]<<<[>>>+>+<<<<-]>>>>[<<<<+>>>>-]+[<+>-]<<<<[-]>>>[<<<+>>>-]<[-]<+>]<-]<[-]>=10.";
        write_file("example.prg", (uint8_t*)example_expr, strlen(example_expr)+1);

        char example_prog[] = {
                MV_A_I, 'A', 0x00, HLT,
        };

        write_file("example2.exe", (uint8_t*)example_prog, sizeof(example_prog));

        for (int i = 0; i < 16; ++i) {
                char num[32];
                itoa(i, num, 16);
                char* str = strcat(num, ".txt");
                char* text = strcat("Hello, World! > ", num);
                write_file(str, text, strlen(text));
                free(str);
                free(text);
        }
*/
        //write_file_system();

        char* kbbuff = malloc(128);

        uint32_t remainingSpace = remaining_heap_space();
        printf("Heap Size Is %d Bytes\n",remainingSpace);
        printf("File Descriptor Size Is %d Bytes\n",sizeof(FileDescriptor));
        
        while (active)
        {
                printf("%c: %s/%s> ", Drive_Letter, ActiveDirParen(), ActiveDir());
                gets(kbbuff, 128);
                system(kbbuff);
        }

        //write_file_system();
        free(kbbuff);
        clearScreen(termCol);
        mode(0x2);
        puts_at("It is now safe to turn of your computer\n", 20, 11);
}
