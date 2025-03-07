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

void system(char* sys)
{
        char cmd[MARGS][CMDM_LEN];
        memset(cmd,0,sizeof(cmd));
        int argc=parse_args(sys,(char (*)[128])&cmd);
        if (strncmp(cmd[0],"./",2)==0)
        {
                char* path = cmd[0]+2;
                while(path[0] == '/')path++;
                int res = ExecuteF(path);
                if (res!=0)
                {
                        r=res;
                }
        }
        else if (strcmp(cmd[0], "info") == 0)
        {
                cli();
                printf("\xff[3f]O\xff[5f]S\xff[7f]A\xff[5f]8\xff[3f]6\xff[r] VERSION %s\n(C) JOSHUA F. 2024-2025\n",__VER__);
                printf("Heap: %dKiB\nRam:  %dMiB\n",remaining_heap_space()/1024,MAX_ADDR/1024/1024);
                sti();
        }
        else if (strcmp(cmd[0], "color") == 0 && argc == 1)
        {
                for (int i = 0; i < 16; ++i)  printf("%c%c", (i < 10) ? (i + '0') : (i + 'A' - 10), (i == 15) ? '\n' : '\0');
                for (int i = 0; i < 16; ++i)  printf("\xff[*b] %c", i, (i == 15) ? '\n' : '\0');
        }
        else if (strcmp(cmd[0], "color") == 0 && argc == 2)
        {
                TTY_COL = 0;
                TTY_COL = strhex(cmd[1]);
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
                cli();
        }
        else if (strcmp(cmd[0], "bf") == 0)
        {
                if (argc == 2) BrainFuck(cmd[1]);
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
        else if (strcmp(cmd[0], "tasks") == 0)
        {
                ListSchedule();
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
                printf("%c:/%s%c%s>", Drive_Letter, ActiveDirParen(), ActiveDirParen()[0]==0 ? '\0' : '/', ActiveDir());
                gets(kbbuff, 128);
                system(kbbuff);
        }
        while(1);
}

#endif