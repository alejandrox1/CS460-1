#ifndef KERNEL_C
#define KERNEL_C

#include "type.h"

// function to create a process DYNAMICALLY
int kfork(char *filename)
{
    // Variables
    int j, i;
    u16 segment;
    
    // Get the new proc
    PROC *p = get_proc(&freeList);
    if (p == 0)
    {
        // if there were no procs, report kfork's failure
        printf("No available free procs\n");
        return 0;
    }
    
    // initialize the proc...
    p->status = READY; // it must be ready to run...
    p->priority = 1; // it has no particular preference on when to run...
    p->ppid = running->pid; // its parent is the current processor, of course!
    p->parent = running;
    
    // now to setup the kstack!
    // first things first, lets clean up the registers by setting them to 0.
    for (j = 1; j < 10; j++)
        p->kstack[SSIZE - j] = 0;
        
    p->kstack[SSIZE - 1] = (int)body; // now we need to make sure to call tswitch from body when the proc runs...
    p->ksp = &(p->kstack[SSIZE - 9]); // set the ksp to point to the top of the stack
    
    // set it up to load umode properly!
    segment = (p->pid + 1) * MTXSEG;
    load(filename, segment);
    for (i=1; i <= 8; i++)         // cpu registers are all 0. was 0-13. less registers
         put_word(0, segment, i * -2);
     printf("segment: %x\n",segment);
     //put_word(WORD, SEGMENT, OFFSET)
     put_word(0x0200,   segment, segment - 2); // still 2
     put_word(segment,  segment, segment - 4); // still 4
     put_word(segment,  segment, segment - 14); // was 14
     put_word(segment,  segment, segment - 16); // was 24

     /* initial USP relative to USS */
     p->uss = segment;
     p->usp = segment - 16; //was 24
    
    // enter the proc into the readyQueue, since it's now ready for primetime!
    enqueue(&readyQueue, p);
    printf("Proc[%d] forked child Proc[%d] at segment %x\n", running->pid, p->pid, segment);
    // return the new proc!!!
    return (p->pid);
}

u8 get_byte(u16 segment, u16 offset){
 	 u8 byte;
 	 setds(segment);
 	 byte = *(u8 *)offset;
 	 setds(MTXSEG);
 	 return byte;
}

int get_word(u16 segment, u16 offset){
	u16 word;
	setds(segment);
	word = *(u16 *)offset;
	setds(MTXSEG);
	return word;
}

int put_byte(u8 byte, u16 segment, u16 offset){
  	setds(segment);
  	*(u8 *)offset = byte;
  	setds(MTXSEG);
}

int put_word(u16 word, u16 segment, u16 offset){
	setds(segment);
	*(u16 *)offset = word;
	setds(MTXSEG);
}

#endif
