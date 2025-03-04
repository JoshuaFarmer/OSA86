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

        init_ramfs();
        if (exists(argv[1]))
        {
                FILE * fp = fopen(argv[1],"rb");
                fread(start,1,65536,fp);
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
                else if (strcmp(argv[i],"/ls") == 0)
                {
                        ListF();
                }
                else if (strcmp(argv[i],"/view") == 0 && i-1 < argc)
                {
                        OSA_FILE *osf = osa_fopen(argv[++i],"r");
                        char *buff = wfread(osf);
                        printf("%s\n",buff);
                        free(buff);
                        osa_fclose(osf);
                }
                else
                {
                        FILE *fp = fopen(argv[i],"r");
                        if (!fp) return -1;
                        fseek(fp,0,SEEK_END);
                        int len = ftell(fp);
                        fseek(fp,0,SEEK_SET);
                        char *x = malloc(len);
                        if (!x) return -1;
                        fread(x,1,len,fp);
                        printf("data:%s\n",x);
                        OSA_FILE *f = osa_fopen(argv[i],"w");
                        osa_fwrite(x,1,len,f);
                        osa_fclose(f);
                        free(x);
                        fclose(fp);
                }
        }
        
        FILE * out = fopen(argv[1],"wb");
        fwrite(start,1,65536,out);
        fwrite(FAT0,1,sizeof(FAE)*MCC,out);
        fwrite(FDS0,1,sizeof(FileDescriptor)*MFC,out);
        fclose(out); out = NULL;
}
