#ifndef INT_C
#define INT_C

#include "type.h"

/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
int kcinth()
{
   	int a,b,c,d, r;
//==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 
    a = get_word(running->uss, running->usp + 26);
    b = get_word(running->uss, running->usp + 28);
    c = get_word(running->uss, running->usp + 30);
    d = get_word(running->uss, running->usp + 32);
	
   switch(a)
   {
        case 0 : r = kgetpid();                break;
        case 1 : r = kpd();                    break;
        case 2 : r = kchname(b);               break;
        case 3 : r = kkmode();                 break;
        case 4 : r = ktswitch();               break;
        case 5 : r = kkwait(b);                break;
        case 6 : r = kkexit(b);                break;
        case 7 : r = kkfork(b);                break;
        case 8 : r = kexec(b);                 break;
       
       
        // FOCUS on ksin() nd ksout()
        case 9 : r = ksout(b);        break;
        case 10: r = ksin(b);         break;
       
       
        case 30 : r = kpipe(b);               break;
        case 31 : r = read_pipe(b,c,d);       break;
        case 32 : r = write_pipe(b,c,d);      break;
        case 33 : r = close_pipe(b);          break;
        case 34 : r = pfd();                  break;

       case 90: r = getc();                   break;
       case 91: r = putc(b);                  break;
       case 99: kkexit(b);                    break;
       default: printf("invalid syscall # : %d\n", a); 
   }

//==> WRITE CODE to let r be the return value to Umode
    put_word(r, running->uss, running->usp + 16);
    return r;
}

//============= WRITE C CODE FOR syscall functions ======================

int kkmode()
{
    body();
}

int kexec(char* filename)
{
    int i, regster;
	u16 segment = (running->pid +1) * 0x1000;
	
	// load the file
    load(getstring(filename), segment);
    
    for (i = 1; i < 13; i++)
    {
        switch (i)
        {
            case 1: regster = 0x0200; break;
            case 10: put_word(0, segment, 0x1000 - i * 2); continue;
            case 2: case 11: case 12: regster = segment; break;
            default: regster = 0; break;
        }
        put_word(regster, segment, 0x1000 - i * 2);
    }
    
    running->uss = segment;
    running->usp = 0x1000 - 24;
    put_word(0, segment, running->usp + 16);
    return 0;
}

// returns the current process id
int kgetpid()
{
    return running->pid;
}

// WRITE C code to print PROC information
int kpd()
{
    int i, count;
    bool showmoreinfo = false;
    bool showevent = false;
    bool showexitcode = false;
    PROC *p;
    
    printf("=======================================================================\n");
    printf("   name       status       pid       ppid       event       exitcode\n");
    printf("-----------------------------------------------------------------------\n");
    
    for (i = 0; i < NPROC; i++)
    {   // initialize all procs
        showmoreinfo = true;
        showevent = false;
        showexitcode = false;
        p = &proc[i];
        count = 14 - strlen(p->name);
        printf("%s", p->name);
        while (count-- > 0)
            printf(" ");
        
        // write the status and set the information vars!
        switch (p->status)
        {
            case FREE:
                printf("FREE         ");
                showmoreinfo = false;
                showexitcode = true;
                break;
            case READY:
                printf("READY        ");
                break;
            case RUNNING:
                printf("RUNNING      ");
                break;
            case STOPPED:
                printf("STOPPED      ");
                showexitcode = true;
                break;
            case SLEEP:
                printf("SLEEP        ");
                showevent = true;
                break;
            case ZOMBIE:
                printf("ZOMBIE       ");
                showevent = true;
                break;
            case BLOCK:
                printf("BLOCK        ");
                showevent = true;
                break;
        }
        
        // show pid and ppid?
        if (showmoreinfo == true)
            printf("%d         %d         ", p->pid, p->ppid);
        else
            printf("                    ");
            
        // show event num?
        if (showevent == true)
            printf("%d             ", p->event);
        else
            printf("              ");
        
        // show exit code?
        if (showexitcode == true)
            printf("%d\n", p->exitCode);
        else
            printf("\n");
    }
    printf("-----------------------------------------------------------------------\n");
    return true;
}

// changes a process name
int kchname(char *name)
{
    char c;
    int i = 0;

    while (i < 32)
    {
        c = get_byte(running->uss, name + i);
        running->name[i] = c;
        if (c == '\0') { break; }
        i++;
    }
}

// forks a process
int kkfork()
{
    return kfork();
}

// switches a process
int ktswitch()
{
    int i;
    running->status = READY;
    i = tswitch();
    running->status = RUNNING;
    
    return i;
}

// waits a process
int kkwait(int *status)
{
    int pid, c;
    pid = kwait(&c);
    put_word(c, running->uss, status);
    return pid;
}

// exits a process
int kkexit(int value)
{
    // use your kexit() in LAB3. do NOT let P1 die
    return kexit(value);
}

// get a line from serial port 0; write line to Umode
int ksin(char *y)
{
    int length, i = 0;
    char line[64], *msgp, *msg = "\n\rString? \n\r\007";
    
    // get the message
    msgp = msg;
    while (*msgp)
    {
        bputc(0x3F8, *msgp);
        msgp++;
    }
    
    // get the line from the message
    length = sgetline(line);
    if (length < 0) return -1;
    
    // then write the line
    do
    {
        put_byte(line[i], running->uss, y++);
    } while(line[i] != '\0');
    
    // finally, return the length
    return length;
}

// get line from Umode; write line to serial port 0
int ksout(char *y)
{
    int length, i = 0;
    char line[64], *msgp, *msg = "\n\rInput: \n\r\007";
    
    msgp = msg;
    while (*msgp)
    {
        bputc(0x3F8, *msgp);
        msgp++;
    }
    
    while (*y)
    {
        line[i] = get_word(running->uss, y++);
        i++;
    }
    
    getc();
    msgp = &line[0];
    sputline(msgp);
    return (i - 1);
}

#endif
