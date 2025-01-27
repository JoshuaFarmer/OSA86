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
//#include "osafs.h"
#include "osafs2.h"
//#include "ed.h"
#include "video.h"
#include "user.h"
#include "gui.h"
#include "program.h"
//#include "UpsaCPUEmu/cpu.h"

void system(char* _syscmd) {
        char* syscmd  = strtok(_syscmd, " \0");
        char* syscmd1 = strtok(NULL, ";\0");
        char* syscmd2 = strtok(NULL, "\0");

        static int r=0;
        if (strncmp(syscmd, "./", 2) == 0) {
                char* path = strcat(STR_PATH, syscmd+2);
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
        } else if (strcmp(syscmd, "ed") == 0) {
                //int EdError = ed();
                //puts("EdExitCode : ");
                //printh(EdError);
                putc('\n');
        } else if (strcmp(syscmd, "ls") == 0) {
                //fs_list(super_block);
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
                char* path = strcat(STR_PATH, syscmd1);
                
                ReadF(path,data,len);
                putsn((const char*)data, len);
                free(data);
                free(path);
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
        mode(0x01);

        init_heap();
        init_gdt();
        putc('\n');
        //init_idt();

        pit_init(100);

        system("info");

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
        puts("Heap Size Is "); PRINT_DWORD(remainingSpace);
        puts("File Descriptor Size "); PRINT_WORD(sizeof(FileDescriptor));
        
        // OS MAINLOOP
        while (active) {
                putc(Drive_Letter);
                putc(':');
                puts(ActiveDir());
                puts(STR_PATH);
                puts("> ");

                gets(kbbuff, 128);
                system(kbbuff);
        }

        //write_file_system();
        free(kbbuff);
        clearScreen(termCol);
        mode(0x2);
        puts_at("It is now safe to turn of your computer\n", 20, 11);
}
