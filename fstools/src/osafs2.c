#include <stdio.h>
#include <stdlib.h>
#include "osafs2.h"

int exists(const char *fname)
{
        FILE *file;
        if ((file = fopen(fname, "rb")))
        {
                fclose(file);
                return 1;
        }
        return 0;
}

int main(int argc, char * argv[])
{
        char start[65536];
        if (argc == 1)
        {
                printf("usage: %s <diskImage>\n",argv[0]);
                return 1;
        }

        InitRamFS();
        if (exists(argv[1]))
        {
                FILE * fp = fopen(argv[1],"rb");
                fread(start,1,65536,fp);
                fseek(fp,65536,SEEK_END);
                fread(FAT0,1,sizeof(FAE)*MCC,fp);
                fread(FDS0,1,sizeof(FileDescriptor)*MFC,fp);
                fclose(fp); fp = NULL;
        }

        for (int i = 2; i < argc; ++i)
        {
                if (strcmp(argv[i],"/cd") == 0 && i-1 < argc)
                {
                        Cd(argv[++i]);
                }
                else if (strcmp(argv[i],"/start") == 0 && i-1 < argc)
                {
                        FILE * fp = fopen(argv[++i],"rb");
                        fread(start,1,65536,fp);
                        fclose(fp); fp = NULL;
                }
                else if (strcmp(argv[i],"/rm") == 0 && i-1 < argc)
                {
                        DeleteF(argv[++i]);
                }
                else if (strcmp(argv[i],"/view") == 0 && i-1 < argc)
                {
                        OsaFILE f = Osafgetf(argv[++i],current_path_idx)-1;
                        char buff[FDS0[f].Size+1];
                        buff[FDS0[f].Size]=0;
                        ReadF(argv[i],buff,FDS0[f].Size);
                        printf("%s\n",buff);
                }
                else if (strcmp(argv[i],"/ls") == 0)
                {
                        ListF();
                }
                else
                {
                        CreateF(argv[i]);
                        FILE * fp = fopen(argv[i],"rb");
                        fseek(fp,0,2);
                        int len = ftell(fp);
                        fseek(fp,0,0);
                        char buff[len+1];
                        fread(buff,len,1,fp);
                        WriteF(argv[i],buff,len);
                        fclose(fp); fp = NULL;
                }
        }
        
        FILE * out = fopen(argv[1],"wb");
        fwrite(start,1,65536,out);
        fwrite(FAT0,1,sizeof(FAE)*MCC,out);
        fwrite(FDS0,1,sizeof(FileDescriptor)*MFC,out);
        fclose(out); out = NULL;
}
