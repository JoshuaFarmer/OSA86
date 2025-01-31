#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef struct Task
{
        uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi,eip;
        uint16_t ds,es,fs,gs,cs,ss;
        uint32_t eflags;
        uint8_t stack[8192];
        void * start;
        int tick;
        struct Task * next;
        bool running;
        char * name;
} Task;

Task RootTask; // on init, switch self to this, so that it's only called on interrupts
Task * ActiveTask=&RootTask;

void Int80(int,int);
void SystemTick();

void test()
{
        putc('2');
        while (true)
        {
        }
}

void test2()
{
        putc('3');
        while (true)
        {
        }
}

void init_scheduler()
{
        memset(&RootTask, 0, sizeof(Task));
        RootTask.next=NULL;
        RootTask.running=true;
        RootTask.eip=(uint32_t)0;
        RootTask.cs=0x8;
        RootTask.ds=0x10;
        RootTask.ss=0x10;
        RootTask.es=0x10;
        RootTask.fs=0x10;
        RootTask.gs=0x10;
        RootTask.eflags=0x200;
        RootTask.name = "Scheduler Root";
        RootTask.esp = (uint32_t)&RootTask.stack[sizeof(RootTask.stack) - 4];
        printf("SCHED Initialized\n");
}

/* we delete them later */
void MarkDead()
{
        ActiveTask->running=false;
}

void AppendTask(char * name, void (*start)(void))
{
        Task * new = malloc(sizeof(Task));
        if (!new) return; // outta ram bitch
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

void ListSchedule()
{
        printf("SCHEDULE:\n");
        IterateSchedule(i)
        {
                putsn(current->name,64); putc('\n');
        }
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

void Scheduler(
                uint32_t * eax, uint32_t * ebx,
                uint32_t * ecx, uint32_t * edx,
                uint32_t * esi, uint32_t * edi,
                uint32_t * ebp, uint32_t * esp,
                uint32_t * eflags, uint32_t * ds,
                uint32_t * ss, uint32_t * es,
                uint32_t * fs, uint32_t * gs,
                uint32_t * eip, uint32_t * cs,
                uint32_t tick
              )
{
        cli();
        if (ActiveTask)
        {
                ActiveTask->eax=*eax;
                ActiveTask->ebx=*ebx;
                ActiveTask->ecx=*ecx;
                ActiveTask->edx=*edx;
                ActiveTask->esi=*esi;
                ActiveTask->edi=*edi;
                ActiveTask->ebp=*ebp;
                ActiveTask->esp=*esp;
                ActiveTask->eip=*eip;
                ActiveTask->eflags=*eflags;
                ActiveTask->ds=*ds;
                ActiveTask->ss=*ss;
                ActiveTask->es=*es;
                ActiveTask->fs=*fs;
                ActiveTask->gs=*gs;
                ActiveTask->cs=*cs;
                ActiveTask->tick++;

                Next();
                *eax = ActiveTask->eax;
                *ebx = ActiveTask->ebx;
                *ecx = ActiveTask->ecx;
                *edx = ActiveTask->edx;
                *esi = ActiveTask->esi;
                *edi = ActiveTask->edi;
                *esp = ActiveTask->esp;
                *ebp = ActiveTask->ebp;
                *eip = ActiveTask->eip;
                *cs  = ActiveTask->cs;
                *ds  = ActiveTask->ds;
                *es  = ActiveTask->es;
                *fs  = ActiveTask->fs;
                *gs  = ActiveTask->gs;
        }
}

void Schedule(const char * path)
{
}

#endif
