#ifndef IL_H
#define IL_H

char id[256];
int  tok=0;
int  num=0;

char * src;

#define iside(x) ((x >= 'A' && x <= 'Z')||(x >= 'a' && x <= 'z')||x=='_')
#define isnum(x) (x >= '0' && x <= '9')

enum
{
        TOK_NONE,
        TOK_NUM,
        TOK_PRINT,
        TOK_SEMI,
        TOK_STR,
        TOK_EX,
        TOK_IDE,
};

typedef struct StackElem
{
        uint32_t x;
        uint32_t type;
} StackElem;

StackElem stack[32];
uint8_t   idestack[8][32];
uint8_t   stackptr=0;
uint8_t   idesp=0;
int       inShell = 0;

void pushide()
{
        memcpy(idestack[idesp],id,32);
        idesp = (idesp + 1) % 8;
}

void popide()
{
        idesp = (idesp - 1) % 8;
        memcpy(id,idestack[idesp],32);
}

void push(int x, int type)
{
        stack[stackptr].x=x;
        stack[stackptr].type=type;
        stackptr = (stackptr + 1) % 32;
}

StackElem pop()
{
        stackptr = (stackptr - 1) % 32;
        StackElem x = stack[stackptr];
        return x;
}

typedef struct Variable
{
        int type;
        union
        {
                int    integer;
                char * str;
        } value;
        char name[32];
        struct Variable * next;
} Variable;

enum variable_type
{
        NONE,
        INTEGER,
        STRING,
        STRINGLIT,
};

Variable * vars = NULL;

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

        Variable * new=NULL;
        if (vars==NULL)
        {
                vars = malloc(sizeof(Variable));
                vars->type=NONE;
                strcpy(vars->name,name);
                vars->next=NULL;
                new=vars;
        }
        else
        {
                new = malloc(sizeof(Variable));
                strcpy(new->name,name);
                new->next = vars->next;
                new->type = NONE;
                vars->next = new;
        }
        return new;
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
                prv=var;
                var = var->next;
        }
        vars=NULL;
}

void next()
{
        num=0;
        while (*src == ' ' || *src == '\n' || *src == '\r') ++src;
        if (*src == '\'')
        {
                size_t x=0;
                memset(id,0,sizeof(id));
                ++src;
                while (*src != '\'' && x < sizeof(id))
                {
                        if (*src == '\\')
                        {
                                ++src;
                                id[x++] = *src;
                        }
                        else
                        {
                                id[x++] = *src;
                        }
                        ++src;
                }
                ++src;
                tok = TOK_STR;
                return;
        }
        else if (iside(*src))
        {
                int x=0;
                memset(id,0,sizeof(id));
                while (iside(*src))
                {
                        id[x++]=*(src++);
                }
                if (strcmp(id,"print")==0)
                {
                        tok=TOK_PRINT;
                }
                else if (strcmp(id,"exit")==0)
                {
                        tok=TOK_EX;
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
                case TOK_STR:
                {
                        char * x = strdup(id);
                        push((uint32_t)x, STRINGLIT);
                } break;
                case TOK_SEMI:
                {
                        return;
                } break;
                case TOK_IDE:
                {
                        pushide();
                        Variable * x = find_var(id);
                        if (x)
                        {
                                switch (x->type)
                                {
                                        case INTEGER:
                                        {
                                                push(x->value.integer, INTEGER);
                                                break;
                                        }
                                        case STRING:
                                        {
                                                push((uint32_t)x->value.str, STRING);
                                                break;
                                        }
                                }
                        }
                } break;
                case TOK_NUM:
                {
                        push(num, INTEGER);
                } break;
                case '=':
                {
                        popide();
                        char name[sizeof(id)];
                        strcpy(name,id);
                        expr();
                        Variable * x = find_var(name);
                        StackElem stc = pop();
                        switch (stc.type)
                        {
                                case INTEGER:
                                {
                                        x->type=INTEGER;
                                        x->value.integer=stc.x;
                                        return;
                                } break;
                                case STRINGLIT:
                                {
                                        x->type=STRING;
                                        x->value.str=(char *)stc.x;
                                } break;
                                case STRING:
                                {
                                        x->type=STRING;
                                        x->value.str=strdup((char *)stc.x);
                                        return;
                                } break;
                        }
                } break;
                case '+':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(a.x+b.x,INTEGER);
                        else if ((a.type == STRING || a.type == STRINGLIT) && (b.type == STRING || b.type == STRINGLIT))
                        {
                                char * new = malloc(strlen((char*)a.x)+strlen((char*)b.x)+1);
                                strcpy(new,(char*)b.x);
                                strcpy(new+strlen((char*)b.x),(char*)a.x);
                                new[strlen((char*)a.x)+strlen((char*)b.x)] = 0;
                                push((int)new,STRINGLIT);
                        }
                        return;
                } break;
                case '-':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(b.x-a.x,INTEGER);
                        return;
                } break;
                case '*':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(a.x*b.x,INTEGER);
                        return;
                } break;
                case '/':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(b.x/a.x,INTEGER);
                        return;
                } break;
                case '&':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(a.x&b.x,INTEGER);
                        return;
                } break;
                case '|':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(a.x|b.x,INTEGER);
                        return;
                } break;
                case '^':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(a.x^b.x,INTEGER);
                        return;
                } break;
                case '!':
                {
                        expr();
                        StackElem a = pop();
                        if (a.type == INTEGER)
                                push(!a.x,INTEGER);
                        return;
                } break;
                case '%':
                {
                        expr();
                        StackElem a = pop();
                        StackElem b = pop();
                        if (a.type == INTEGER && b.type == INTEGER)
                                push(b.x%a.x,INTEGER);
                        return;
                } break;
                case TOK_EX:
                {
                        inShell = 0;
                        return;
                } break;
                case TOK_PRINT:
                {
                        expr();
                        StackElem x = pop();
                        if (x.type == INTEGER)
                        {
                                printf("%d\n",x.x);
                        }
                        else if (x.type == STRING || x.type == STRINGLIT)
                        {
                                printf("%s\n",(char *)x.x);
                        }
                        else if (x.type == NONE)
                        {
                                printf("None\n");
                        }
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
                for (int i = 0; i < 256; ++i)
                {
                        if (stack[i].type == STRINGLIT)
                        {
                                free((char*)stack[i].x);
                        }
                }
                memset(stack,0,sizeof(stack));
                stackptr=0;
                expr();
        }
        for (int i = 0; i < 256; ++i)
        {
                if (stack[i].type == STRINGLIT)
                {
                        free((char*)stack[i].x);
                }
        }
}

void shell()
{
        printf("OK\n");
        char kbbuff[128];
        inShell = 1;
        while (inShell)
        {
                printf(">");
                gets(kbbuff, 128);
                Interpreter(kbbuff);
        }
        cleanup();
}

#endif