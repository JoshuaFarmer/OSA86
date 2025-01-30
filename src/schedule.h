#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef struct Task
{
        uint32_t eax,ecx,edx,ebx,esp,ebp,esi,edi,eip;
        uint16_t ds,es,fs,gs,cs;
        uint32_t flags;
        struct Task * next;
        bool running;
} Task;

Task RootTask; // on init, switch self to this, so that it's only called on interrupts
Task * ActiveTask=&RootTask;

void InitScheduler()
{
        RootTask.next=NULL;
        RootTask.running=false;
}

/* we delete them later */
void MarkDead()
{
        ActiveTask->running=false;
}

void AppendTask()
{
        Task * new = malloc(sizeof(Task));
        if (!new) return; // outta ram bitch
        new->next=RootTask.next;
        RootTask.next=new;
}

void Scheduler()
{
}

void Schedule(const char * path)
{
}

#endif
