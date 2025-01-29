#ifndef SCHEDULE_H
#define SCHEDULE_H

void Schedule(const char * path)
{
        // TODO : Create linked list of programs in user space.
        /* Tick Timer :
         | EVEN SYSTEM
         | ODD  USERPROGRAM
         | For instance
         | tickerHandle()
         | {
         |   ++x
         |   if (x & 1)
         |   {
         |     save()
         |     currentTask=currentTask->next
         |     load()
         |   }
         |   else
         |     systemTick()
         | }
        */
}

#endif
