#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef uint32_t REGS[16];

enum REG
{
        EAX,
        EBX,
        ECX,
        EDX,
        ESP,
        EBP,
        ESI,
        EDI,
        EIP,
        CS,
        DS,
        ES,
        SS,
        FS,
        GS,
        EFLAGS,
};

typedef struct Task
{
        uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi,eip;
        uint16_t ds,es,fs,gs,cs,ss;
        uint32_t eflags;
        uint8_t stack[8192*4];
        void * start;
        int tick;
        struct Task * next;
        bool running;
        char * name;
} Task;

Task RootTask,*ActiveTask;

volatile uint32_t *regs = (uint32_t *)0xFF00;

void Int80(int,int);
void SystemTick();

void init_scheduler()
{
        memset(&RootTask, 0, sizeof(Task));
        RootTask.next=NULL;
        RootTask.running=true;
        RootTask.eip=(uint32_t)init;
        RootTask.cs=0x8;
        RootTask.ds=0x10;
        RootTask.ss=0x10;
        RootTask.es=0x10;
        RootTask.fs=0x10;
        RootTask.gs=0x10;
        RootTask.eflags=0x200;
        RootTask.name = "osa86";
        RootTask.esp = (uint32_t)&RootTask.stack[sizeof(RootTask.stack) - 4];
        ActiveTask=&RootTask;
        printf("SCHED Initialized\n");
}

/* we delete them later */
void MarkDead()
{
        ActiveTask->running=false;
}

void AppendTask(char * name, void (*start)(void))
{
        if (!name || !start) return;
        Task * new = malloc(sizeof(Task));
        if (!new) {
                printf("outta ram bitch\n");
                return;
        }
        memset(new, 0, sizeof(Task));
        new->next=RootTask.next;
        RootTask.next=new;
        new->name=strdup(name);
        new->start=(void*)start;
        new->cs=0x8;
        new->ds=0x10;
        new->ss=0x10;
        new->es=0x10;
        new->fs=0x10;
        new->gs=0x10;
        new->eflags=0x200;
        new->esp = (uint32_t)&new->stack[sizeof(new->stack) - 4];
        new->eip = (uint32_t)start;
        new->running = true;
}

int ScheduleLength()
{
        Task * current = &RootTask;
        int c=0;
        while (current != NULL)
        {
                ++c;
                current=current->next;
        }

        return c;
}

#define IterateSchedule(_) int _=0; for (Task * current = &RootTask; current != NULL; current = current->next,++_)

int abs(int x)
{
        return x>0 ? x : -x;
}

uint32_t temp1,temp2,temp3;

extern void StackDump();

void StackDump()
{
        asm ("movl %esp, %eax"); 
        asm ("addl $4,%eax"); 
        uint32_t esp; asm volatile ("movl %%eax, %0" : "=r" (esp)); 
        cli();
        clearScreen(0x1F);

        int Size=64;
        uint32_t * data = (uint32_t*)esp;
        printf("ESP = %x\n", esp);
        printf("ESP -%w: ", (Size>>1));
        for (int i = 1; i <= Size; ++i)
        {
                int pos=i-(Size>>1);
                PRINT_DWORD_NE(data[pos]); putc(' ');
                if ((i % 4) == 0 && i < Size)
                {
                        putc('\n');
                        printf("ESP %c%w: ", pos > 0 ? '+' : (pos<0 ? '-' : ' '), abs(pos));
                }
        }

        putc('\n');
}

uint32_t rol(uint32_t value, uint32_t shift)
{
    return (value << shift) | (value >> (32 - shift));
}

uint32_t ror(uint32_t value, uint32_t shift)
{
    return (value >> shift) | (value << (32 - shift));
}

int Name2PID(const char *s, int i)
{
        int res=0;
        for (int u = 0; u < strlen(s); ++u)
        {
                res = res + (s[u] * u) - i;
        }

        if (i & 1)
                return rol(res,(int)s) / 100;
        else
                return ror(res,(int)s) / 100;
}

void PKill(int id)
{
        IterateSchedule(jd)
        {
                if (current && Name2PID(current->name,jd) == id)
                {
                        current->running=false;
                }
        }
}

void ListSchedule()
{
        IterateSchedule(i)
        {
                if (current)
                {
                        printf("%s (%d)\t\t",current->name,Name2PID(current->name,i));
                        if (((i+1) % 4) == 0 && i != 0)
                        {
                                putc('\n');
                        }
                }
        }

        putc('\n');
}

void LookForDead()
{
        Task * prev    = NULL;
        Task * current = &RootTask;

        while (current != NULL)
        {
                if (!current->running && current != &RootTask)
                {
                        if (prev)
                        {
                                prev->next = current->next;
                        }
                        prev = current;
                        Task * to_free = current;
                        current = current->next;
                        free(to_free->name);
                        free(to_free->start);
                        free(to_free);
                }
                else
                {
                        prev = current;
                        current = current->next;
                }
        }
}

void Next()
{
        if (ActiveTask && ActiveTask->next)
        {
                ActiveTask = ActiveTask->next;
                while (ActiveTask && !ActiveTask->running)
                {
                        ActiveTask = ActiveTask->next;
                        if (!ActiveTask) ActiveTask = &RootTask;
                }
        }
        else if (!ActiveTask->next)
        {
                ActiveTask = &RootTask;
        }
}

void Scheduler()
{
        cli();
        if (ActiveTask)
        {
                ActiveTask->eax   =regs[EAX];
                ActiveTask->ebx   =regs[EBX];
                ActiveTask->ecx   =regs[ECX];
                ActiveTask->edx   =regs[EDX];
                ActiveTask->esi   =regs[ESI];
                ActiveTask->edi   =regs[EDI];
                ActiveTask->ebp   =regs[EBP];
                ActiveTask->esp   =regs[ESP];
                ActiveTask->eip   =regs[EIP];
                ActiveTask->eflags=regs[EFLAGS];
                ActiveTask->ds    =regs[DS];
                ActiveTask->ss    =regs[SS];
                ActiveTask->es    =regs[ES];
                ActiveTask->fs    =regs[FS];
                ActiveTask->gs    =regs[GS];
                ActiveTask->cs    =regs[CS];
                ActiveTask->tick++;

                Next();
                regs[EAX]    = ActiveTask->eax;
                regs[EBX]    = ActiveTask->ebx;
                regs[ECX]    = ActiveTask->ecx;
                regs[EDX]    = ActiveTask->edx;
                regs[ESI]    = ActiveTask->esi;
                regs[EDI]    = ActiveTask->edi;
                regs[ESP]    = ActiveTask->esp;
                regs[EBP]    = ActiveTask->ebp;
                regs[EIP]    = ActiveTask->eip;
                regs[EFLAGS] = ActiveTask->eflags;
                regs[CS]     = ActiveTask->cs;
                regs[DS]     = ActiveTask->ds;
                regs[SS]     = ActiveTask->ss;
                regs[ES]     = ActiveTask->es;
                regs[FS]     = ActiveTask->fs;
                regs[GS]     = ActiveTask->gs;
        }
}

/*schedule(path (*)) would just be ExecuteF*/

#endif
