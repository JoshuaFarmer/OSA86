#include<stdio.h>
#include<stdlib.h>

main(argc,argv)
int argc;
char ** argv;
{
        if (argc!=3)return(-2);
        FILE * fp = fopen(argv[1],"rb");
        FILE * fo = fopen(argv[2],"w");
        if (!fp||!fo)return(-2);
        char c;
        fprintf(fo,"char %s[] = {",argv[1]);
        while((c=fgetc(fp))!=EOF)
        {
                fprintf(fo,"0x%x,",c);
        }
        fprintf(fo,"};\nCreateF(\"%s\");WriteF(\"%s\",\"%s\",sizeof(%s));\n",argv[1],argv[1],argv[1],argv[1]);
        fclose(fp);
        fclose(fo);
        fp=fo=NULL;
        return 0;
}