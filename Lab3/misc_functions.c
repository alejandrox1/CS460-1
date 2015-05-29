#ifndef MISCFUNCTIONS_C
#define MISCFUNCTIONS_C

#include "types.h"

bool help()
{
    int i;
    printf("\nAvailable Commands: \n");
    for (i = 0; commands[i].key; i++)
    {
        printf(" - %c: %s: %s\n", commands[i].key, commands[i].name, commands[i].help);
    }
    return true;
}

bool shorthelp()
{
    int i;
    printf("Enter a char [");
    
    for (i = 0; commands[i].key; i++)
    {
        printf(" %c ", commands[i].key);
        if (commands[i + 1].key != null)
        {
            printf("|");
        }
    }
    
    printf("] : ");
    return true;
}

// Initialize the OS
int init()
{
       PROC *p;
       int i;

       printf("init ....");

       for (i=0; i<NPROC; i++){   // initialize all procs
           p = &proc[i];
           p->pid = i;
           p->status = FREE;
           p->priority = 0;     
           p->next = &proc[i+1];
       }
       freeList = &proc[0];      // all procs are in freeList
       proc[NPROC-1].next = 0;
       readyQueue = sleepList = 0;

       /**** create P0 as running ******/
       p = get_proc(&freeList);
       p->status = RUNNING;
       running = p;
       nproc++;
       printf("done\n");
}

// MENU INTERFACE!
int body()
{
    int i, out;
    char c;
    printf("proc %d resumes to body()\n", running->pid);
    while(1)
    {
        printf("============================================\n");
        printf("freeList: ");
        printQueue(freeList);
        printf("readyQueue: ");
        printQueue(readyQueue);
        printf("sleepList: ");
        printQueue(sleepList);
        printf("--------------------------------------------\n");

        printf("\nproc %d[%d] running: parent=%d\n",
        running->pid, running->priority, running->ppid);

        shorthelp();
        c = getc(); printf("%c\n", c);

        for (i = 0; commands[i].key; i++)
        {
            if (c == commands[i].key)
            {
                printf("Executing command %s...\n", commands[i].name);
                out = commands[i].f();
                printf("\nCommand finished!\n\n");
            }
        }
    }
}

#endif
