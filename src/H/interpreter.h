#ifndef IL_H
#define IL_H

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
        char * end;
} Subroutine;

int stack[1024];
int stackptr=0;

void push(int x)
{
        stack[stackptr++]=x;
}

int pop()
{
        return stack[--stackptr];
}

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
                if (strcmp(id,"sub")==0)
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

void expr()
{
        next();
        switch (tok)
        {
                case TOK_NUM:
                {

                }
        }
}

void Interpreter(char * s)
{
        src=s;
        while (tok != 0)
        {
                expr();
                printf("TOK='%c',ID='%s',N=%d\n",tok,id,num);
        }
}

#ifndef _OSA86

int main()
{
        JCompiler("FN MAIN");
}

#endif

#endif