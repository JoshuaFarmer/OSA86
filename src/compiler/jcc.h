char id[32];
int  tok=0;
int  num=0;

char * src;

#define iside(x) ((x >= 'A' && x <= 'Z')||(x >= 'a' && x <= 'z'))
#define isnum(x) (x >= '0' && x <= '9')

enum
{
        TOK_IDE='I',
        TOK_FN='F',
        TOK_NUM='N',
};

typedef struct
{
        char   name[32];
        char * start;
} Function;

void next()
{
        num=0;
        while (*src == ' ' || *src == '\n' || *src == '\r') ++src;
        if (iside(*src))
        {
                int x=0;
                while (iside(*src))
                {
                        id[x++]=*(src++);
                }
                if (strcmp(id,"fn")==0)
                {
                        tok=TOK_FN;
                }
                else
                        tok=TOK_IDE;
                return;
        }
        else if (isnum(*src))
        {
                while (isnum(*src))
                {
                        num = num * 10 + (*(src++) - '0');
                }
                tok=TOK_NUM;
                return;
        }
        tok = *(src++);
}

void JCompiler(char * s)
{
        src=s;
        next();
        while (tok != 0)
        {
                printf("TOK='%c',ID='%s',N=%d\n",tok,id,num);
                next();
        }
}

#ifndef _OSA86

int main()
{
        JCompiler("FN MAIN");
}

#endif
