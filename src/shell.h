#ifndef SHELL_H
#define SHELL_H

#include "mlmon.h"
#include "string.h"

#define CMDM_LEN 128
#define MARGS 32

int parse_args(b,cmd)
        char * b,cmd[MARGS][CMDM_LEN];
{
        int N,i;
        for (N=0,i=0;*b&&N<MARGS&&i<CMDM_LEN;cmd[N++][i]=0,++b,i=0)
        {
                while (*b == ' ') b++;
                if (*b=='"'){for(++b;*b!='"'&&*b;++b,++i)
                        cmd[N][i]=(*b=='\\')?((*(++b)=='n')?'\n':*b):*b;}
                else for (;*b&&*b!=' ';cmd[N][i++]=*(b++));
        }
        return N;
}

extern void test_custom_opcodes(void);
void PKill(int);

void system(char* sys)
{
        char cmd[MARGS][CMDM_LEN];
        memset(cmd,0,sizeof(cmd));
        int argc=parse_args(sys,(char (*)[128])&cmd);
        if (strncmp(cmd[0],"./",2)==0)
        {
                char* path = cmd[0]+2;
                while(path[0] == '/')path++;
                int pid = ExecuteF(path);
                if (pid == -1) return;
                while (1)
                {
                        if (!ProcessIsActive(pid)) {PKill(pid); return;}
                        /* CTRL+C */
                        if (getc() == ('C'-'@')) {PKill(pid); LookForDead(); return;}
                }
        }
        else if (strcmp(cmd[0], "opcode") == 0)
        {
                test_custom_opcodes();
        }
        else if (strcmp(cmd[0], "info") == 0)
        {
                printf("\xff[*f]O\xff[*f]S\xff[*f]A\xff[*f]8\xff[*f]6\xff[r] VERSION %s\n(C) JOSHUA F. 2024-2025\n",0b11,0b110101,0b111111,0b110101,0b11,__VER__);
                printf("Heap: %dKiB\nRam:  %dMiB\n",remaining_heap_space()/1024,MAX_ADDR/1024/1024);
        }
        else if (strcmp(cmd[0], "yield") == 0 && argc == 2)
        {
                int pid = atoi(cmd[1]);
                while (1)
                {
                        if (!ProcessIsActive(pid)) {PKill(pid); return;}
                        /* CTRL+C */
                        if (getc() == ('C'-'@')) {PKill(pid); LookForDead(); return;}
                }
        }
        else if (strcmp(cmd[0], "colour") == 0 && argc == 1)
        {
                int colour = 0;
                for (int y = 0; y < (64/16)+2; ++y)
                {
                        for (int x = 0; x < (16); ++x)
                        {
                                printf("\xff[*b] ",colour++);
                        }
                        printf("\n");
                }
        }
        else if (strcmp(cmd[0], "KillEmAll") == 0)
        {
                IterateSchedule(i)
                {
                        if (i > 2)
                        {
                                printf("ending %s\n", current->name);
                                PKill(i);
                        }
                }
        }
        else if (strcmp(cmd[0], "colour") == 0 && argc == 3)
        {
                TTY_COL  = strhex(cmd[1]);
                TTY_COL |= strhex(cmd[2]) << 8;
                clearScreen(TTY_COL);
        }
        else if (strcmp(cmd[0], "&") == 0 && argc>1)
        {
                int i;
                for (i=1;i<argc;++i)
                {
                        ExecuteF(cmd[i]);
                }
                ListSchedule();
        }
        else if (strcmp(cmd[0], "cls") == 0)
        { 
                clearScreen(TTY_COL);
        }
        else if (strcmp(cmd[0], "?") == 0)
        { 
                printf("%d\n",r);
        }
        else if (strcmp(cmd[0], "ls") == 0)
        {
                ListF();
        }
        else if (strcmp(cmd[0], "cd") == 0 && argc==2)
        {
                Cd(cmd[1]);
        }
        else if (strcmp(cmd[0], "shutdown") == 0)
        {
                active = false;
        }
        else if (strcmp(cmd[0], "pKill") == 0 && argc == 2)
        {
                int i = atoi(cmd[1]);
                PKill(i);
        }
        else if (strcmp(cmd[0], "view") == 0 && argc == 2)
        {
                FILE *fp = fopen(cmd[1],"r");
                char *data = wfread(fp);
                printf("%s",data);
                free(data);
                fclose(fp);
        }
        else if (strcmp(cmd[0], "write") == 0 && argc == 3)
        {
                WriteF(cmd[2],cmd[1],strlen(cmd[1]));
        }
        else if (strcmp(cmd[0], "create") == 0 && argc == 2)
        {
                CreateF(cmd[1]);
        }
        else if (strcmp(cmd[0], "rm") == 0 && argc == 2)
        {
                DeleteF(cmd[1]);
        }
        else if (strcmp(cmd[0], "tasks") == 0 && argc == 1)
        {
                ListSchedule();
        }
        else if (strcmp(cmd[0], "tasks") == 0 && argc == 2 && strcmp(cmd[1], "-pid") == 0)
        {
                ListPID();
        }
        else if (strcmp(cmd[0], "mlmon") == 0 && argc == 2)
        {
                mlmon(cmd[1]);
        }
        else if (strcmp(cmd[0], "time") == 0)
        {
                rtc_get_time(&hour, &minute, &second);
                rtc_get_date(&day, &month, &year);
                printf("Time: %d:%d:%d, ", hour, minute, second);
                printf("Date: %d/%d/%d\n", day, month, year);
        }
        else if (cmd[0][0])
        {
                _ExecuteF(cmd[0],-1);
        }
}

void shell()
{
        char kbbuff[128];
        do
        {
                /* Shell Mainloop */
                memset(kbbuff,0,sizeof(kbbuff));
                printf("/%s%c%s>", ActiveDirParen(), ActiveDirParen()[0]==0 ? '\0' : '/', ActiveDir());
                gets(kbbuff, 128);
                system(kbbuff);
        }
        while(1);
}

#endif