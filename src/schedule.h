#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef struct Task
{
        uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi,eip;
        uint16_t ds,es,fs,gs,cs,ss;
        uint32_t eflags;
        struct Task * next;
        bool running;
        char * name;
} Task;

Task RootTask; // on init, switch self to this, so that it's only called on interrupts
Task * ActiveTask=&RootTask;

void RootTaskMain()
{
        puts("ROOT TASK");
        while(1)
        {
        }
}

void init_scheduler()
{
        memset(&RootTask, 0, sizeof(Task));
        RootTask.next=NULL;
        RootTask.running=true;
        RootTask.eip=(uint32_t)RootTaskMain;
        RootTask.cs=0x8;
        RootTask.name = "Scheduler Root";
        printf("SCHED Initialized\n");
}

/* we delete them later */
void MarkDead()
{
        ActiveTask->running=false;
}

void AppendTask(char * name)
{
        Task * new = malloc(sizeof(Task));
        if (!new) return; // outta ram bitch
        new->next=RootTask.next;
        RootTask.next=new;
        memset(new, 0, sizeof(Task));
        new->name=name;
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

void StackDump()
{
        uint32_t esp; asm volatile ("movl %%esp, %0" : "=r" (esp)); 
        cli();
        clearScreen(0x1F);

        int Size=128;
        uint8_t * data = (uint8_t*)esp;
        printf("ESP -%w: ", (Size>>1));
        for (int i = 1; i <= Size; ++i)
        {
                int pos=i-(Size>>1);
                PrintByte(data[pos]); putc(' ');
                if ((i % 16) == 0 && i < Size)
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
                        current = current->next;
                        free(current);
                }
                else
                {
                        prev = current;
                        current = current->next;
                }
        }
}

static void Next()
{
        if (ActiveTask && ActiveTask->next)
        {
                ActiveTask = ActiveTask->next;
                if (!ActiveTask->running)
                {
                        Next();
                }
        }
        else if (!ActiveTask->next)
        {
                ActiveTask = &RootTask;
        }
}

void Scheduler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp, uint32_t esp, uint32_t eflags, uint32_t ds, uint32_t ss, uint32_t es, uint32_t fs, uint32_t gs, uint32_t eip, uint16_t cs)
{
        if (ActiveTask)
        {
                ActiveTask->eax=eax;
                ActiveTask->ebx=ebx;
                ActiveTask->ecx=ecx;
                ActiveTask->edx=edx;
                ActiveTask->esi=esi;
                ActiveTask->edi=edi;
                ActiveTask->ebp=ebp;
                ActiveTask->esp=esp;
                ActiveTask->eip=eip;
                ActiveTask->eflags=eflags;
                ActiveTask->ds=ds;
                ActiveTask->ss=ss;
                ActiveTask->es=es;
                ActiveTask->fs=fs;
                ActiveTask->gs=gs;
                ActiveTask->cs=cs;

                Next();
        }
}

void Schedule(const char * path)
{
}

#endif
