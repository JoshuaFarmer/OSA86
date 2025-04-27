#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef uint32_t REGS[16];

typedef struct Task
{
        uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi,eip,psuedo_regs[64]; /*r0-r63*/
        uint16_t ds,es,fs,gs,cs,ss;
        uint32_t eflags;
        uint8_t stack[8192];
        void * start;
        int tick;
        int pid;
        struct Task * next;
        bool running;
        char * name;
        int ring0;
} Task;

Task RootTask,*ActiveTask;

volatile uint32_t *regs = (uint32_t *)0xFF00;

void Int80(int,int);
void SystemTick();

void idk()
{
        while(true)
        {
                sti();
        }
}

void init_scheduler()
{
        memset(&RootTask, 0, sizeof(Task));
        RootTask.ring0=true;
        RootTask.next=NULL;
        RootTask.running=true;
        RootTask.eip=(uint32_t)idk;
        RootTask.cs=0x8;
        RootTask.ds=0x10;
        RootTask.ss=0x10;
        RootTask.es=0x10;
        RootTask.fs=0x10;
        RootTask.gs=0x10;
        RootTask.pid=0;
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

Task *Tail()
{
        Task *curr = &RootTask;
        while (curr->next)
        {
                curr = curr->next;
        }

        return curr;
}

/* generic */
Task *CreateTask(char *name,void (*start)(void))
{
        static int pid=1;
        if (!name || !start) return NULL;
        Task * new = malloc(sizeof(Task));
        if (!new)
        {
                printf("outta ram bitch\n");
                return NULL;
        }
        memset(new, 0, sizeof(Task));
        new->next=Tail()->next;
        Tail()->next=new;
        new->ring0=false;
        new->name=strdup(name);
        new->start=(void*)start;
        new->eflags=0x200;
        new->esp = (uint32_t)&new->stack[sizeof(new->stack) - 4];
        new->eip = (uint32_t)start;
        new->running = false;
        new->pid = pid++;
        return new;
}

/* ring 3 */
int AppendTask(char * name, void (*start)(void))
{
        Task *tsk = CreateTask(name,start);
        if (!tsk) return -1;
        tsk->cs = 0x18|3;
        tsk->ds = 0x20|3;
        tsk->ss = 0x20|3;
        tsk->es = 0x20|3;
        tsk->fs = 0x20|3;
        tsk->gs = 0x20|3;
        tsk->running = true;
        return tsk->pid;
}

/* ring 0 */
int AppendTaskRing0(char * name, void (*start)(void))
{
        Task *tsk = CreateTask(name,start);
        if (!tsk) return -1;
        tsk->cs=0x8;
        tsk->ds=0x10;
        tsk->ss=0x10;
        tsk->es=0x10;
        tsk->fs=0x10;
        tsk->gs=0x10;
        tsk->ring0 = true;
        tsk->running = true;
        return tsk->pid;
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
        clearScreen(0x003F);

        int Size=32;
        uint32_t * data = (uint32_t*)esp;
        printf("ESP = %x\n", esp);
        printf("ESP -%w: ", (Size>>1));
        for (int i = 1; i <= Size; ++i)
        {
                int pos=i-(Size>>1);
                PRINT_DWORD_NE(data[pos]); putc(' ');
                if ((i % 2) == 0 && i < Size)
                {
                        putc('\n');
                        printf("ESP %c%w: ", pos > 0 ? '+' : (pos<0 ? '-' : ' '), abs(pos));
                }
        }

        putc('\n');
}

extern void timer_interrupt_handler(void);

void PKill(int id)
{
        IterateSchedule(jd)
        {
                if (current && current->pid == id)
                {
                        current->running=false;
                        if (current == ActiveTask)
                        {
                                timer_interrupt_handler();
                        }
                }
        }
}

bool ProcessIsActive(int PID)
{
        IterateSchedule(_)
        {
                if (current && current->pid == PID)
                {
                        return current->running;
                }
        }

        return false;
}

void ListSchedule()
{
        IterateSchedule(i)
        {
                if (current)
                {
                        printf("%s; ",current->name);
                }
        }

        putc('\n');
}

void ListPID()
{
        IterateSchedule(i)
        {
                if (current)
                {
                        printf("%s\t\t%d\n",current->name,current->pid);
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
