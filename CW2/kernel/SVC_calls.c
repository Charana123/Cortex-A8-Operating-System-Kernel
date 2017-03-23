#include "SVC_calls.h"


/*
  Creates a PCB (Process Block) entry for a newly created process. Used during a fork system call for creation of child
  process.

  @param ctx - Context, SVC stack pointer (state of gprs) to be copied to the newly created process.
               Context is that of the parent process.
  @param basePriority - Base priority of the newly created process. Given to child by parent process.
  @param index - Index in the PCB Table that the newly created Process Block exists.
*/
void createPCB(ctx_t* ctx, pcb_t *pcb, int basePriority, int nextFreePCB, int maxProcesses){
  memset( &pcb[ nextFreePCB ], 0, sizeof( pcb_t ) );
  pcb[nextFreePCB].pid = nextFreePCB + 1; // Gives the process an id
  pcb[nextFreePCB].active = 1; // This states the program is active
  pcb[nextFreePCB].basePriority = basePriority;
  pcb[nextFreePCB].effectivePriority = pcb[nextFreePCB].basePriority;
  pcb[nextFreePCB].buffers = NULL;
  pcb[nextFreePCB].nbuffers = 0;

  //Copy Execution Context of Parent of Child
  memcpy( &pcb[ nextFreePCB ].ctx, ctx, sizeof( ctx_t ) );
  pcb[ nextFreePCB ].ctx.sp   = ( uint32_t ) pcb[nextFreePCB-1].ctx.sp + 0x00001000; //Allocate Stack Space
  if(nextFreePCB == maxProcesses) { maxProcesses++; }
}


int svc_fork(int basePriority, ctx_t* ctx, pcb_t *pcb, int maxProcesses){
  //Tries to find an empty PCB(as a result of a killed Process)
  int nextFreePCB; bool found = true;
  for(nextFreePCB = 0; pcb[nextFreePCB].active == 1; nextFreePCB++){
    if(nextFreePCB == maxProcesses) { found = false; break; }
  }
  //If one if found, it is populated with the child process
  if(found == true) { createPCB(ctx, pcb, basePriority, nextFreePCB, maxProcesses); return nextFreePCB; }
  //Else a new PCB is created at the end of the PCB list
  else { createPCB(ctx, pcb, basePriority, nextFreePCB, maxProcesses); return maxProcesses; }
}
