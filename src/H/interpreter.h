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
        TOK_PRINT='P',
        TOK_SET='=',
        TOK_SEMI=';',
};

typedef struct
{
        char   name[32];
        char * start;
        char * end;
} Subroutine;

uint32_t stack[256];
uint8_t  stackptr=0;

uint8_t idestack[32][32];
uint8_t idesp=0;

void pushide()
{
        memcpy(idestack[idesp],id,32);
        idesp = (idesp + 1) % 32;
}

void popide()
{
        idesp = (idesp - 1) % 32;
        memcpy(id,idestack[idesp],32);
}

void push(int x)
{
        stack[stackptr++]=x;
}

int bottom()
{
        return stack[0];
}

int top()
{
        return stack[stackptr-1];
}

int pop()
{
        return stack[--stackptr];
}

typedef struct
{
        int * arr;
        int len;
} Array;

typedef struct Variable
{
        int type;
        union
        {
                int    integer;
                char * str;
                Array  integer_arr;
        } value;
        char name[32];
        struct Variable * next;
} Variable;

enum variable_type
{
        INTEGER,
        STRING,
        ARRAY,
};

Variable * vars = NULL;

Variable * create_var(int type)
{
        Variable * new=NULL;
        if (vars==NULL)
        {
                vars = malloc(sizeof(Variable));
                vars->type=type;
                strcpy(vars->name,id);
                vars->next=NULL;
                vars->value.integer_arr.arr=NULL;
                vars->value.integer_arr.len=0;
                new=vars;
        }
        else
        {
                new = malloc(sizeof(Variable));
                strcpy(new->name,id);
                new->next = vars->next;
                new->type = type;
                new->value.integer_arr.arr=NULL;
                new->value.integer_arr.len=0;
                vars->next = new;
        }
        return new;
}

Variable * find_var(char * name)
{
        Variable * var = vars;
        while (var != NULL)
        {
                if (strcmp(var->name,name) == 0)
                {
                        return var;
                }
                var = var->next;
        }
        return NULL;
}

void cleanup()
{
        Variable * var = vars;
        Variable * prv = NULL;
        while (var != NULL)
        {
                if (prv && prv->type == INTEGER)
                {
                        free(prv);
                }
                else if (prv && prv->type == STRING)
                {
                        free(prv->value.str);
                        free(prv);
                }
                else if (prv && prv->type == ARRAY)
                {
                        free(prv->value.integer_arr.arr);
                        free(prv);
                }
                prv=var;
                var = var->next;
        }
        vars=NULL;
}

void next()
{
        num=0;
        while (*src == ' ' || *src == '\n' || *src == '\r') ++src;
        if (iside(*src))
        {
                int x=0;
                memset(id,0,sizeof(id));
                while (iside(*src))
                {
                        id[x++]=*(src++);
                }
                if (strcmp(id,"sub")==0)
                {
                        tok=TOK_FN;
                }
                else if (strcmp(id,"print")==0)
                {
                        tok=TOK_PRINT;
                }
                else
                {
                        tok=TOK_IDE;
                }
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
                case TOK_SEMI:
                {
                        return;
                } break;
                case TOK_IDE:
                {
                        pushide();
                        Variable * x = find_var(id);
                        if (x != NULL)
                        {
                                push(x->value.integer);
                        }
                        else
                        {
                                push(0);
                        }
                } break;
                case TOK_NUM:
                {
                        push(num);
                } break;
                case '=':
                {
                        popide();
                        expr();
                        Variable * x = find_var(id);
                        while (!x)
                        {
                                x = create_var(INTEGER);
                        }
                        x->type=INTEGER;
                        x->value.integer = pop();
                } break;
                case '+':
                {
                        expr();
                        int a = pop();
                        int b = pop();
                        push(a+b);
                        return;
                } break;
                case '-':
                {
                        expr();
                        int a = pop();
                        int b = pop();
                        push(a-b);
                        return;
                } break;
                case '*':
                {
                        expr();
                        int a = pop();
                        int b = pop();
                        push(a*b);
                        return;
                } break;
                case '/':
                {
                        expr();
                        int a = pop();
                        int b = pop();
                        push(a/b);
                        return;
                } break;
                case TOK_PRINT:
                {
                        expr();
                        printf("%d\n",top());
                } break;
                case 0:
                {
                        return;
                } break;
        }
        expr();
}

void Interpreter(char * s)
{
        src=s;
        tok=1;
        while (tok != 0)
        {
                memset(stack,0,sizeof(stack));
                stackptr=0;
                expr();
        }
        cleanup();
}

#endif