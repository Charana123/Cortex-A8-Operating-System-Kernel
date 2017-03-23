#include "scheduler.h"

/*
  A process ageing methadology.
  The process that ran before an invocation of the schedular - Effective priority is sent back to its base priority.
  Processes that did not run that turn - Effective priority is incremented
*/
void updatePriorities(pcb_t *pcb, int currentProcess, int maxProcesses){
  for(int i=0; i<maxProcesses; i++){
    if(i == currentProcess) { pcb[i] -> effectivePriority = pcb[i] -> basePriority; continue; }
    pcb[i] -> effectivePriority = pcb[i] -> effectivePriority + 1;
  }
}

/*
  Next highest priority process to be scheduled
*/
int nextProcess(pcb_t *pcb, int maxProcesses){
  //Find highest priority process
  int highestPriorityProcess = 0;
  for(int i=0; i<maxProcesses; i++){
    if(pcb[i].active == 0) { continue; } //Skip empty PCB block (dead process)
    if(pcb[i].effectivePriority > pcb[highestPriorityProcess].effectivePriority){
      highestPriorityProcess = i;
    }
  }
  return highestPriorityProcess;
}


void priorityScheduler(ctx_t* ctx, pcb_t *pcb, int currentProcess, int maxProcesses){

  updatePriorities(pcb, currentProcess, maxProcesses);
  /* Saves curent Execution Context from the SVC Stack (Register State + PC + LR + CSPR)
     in the PCB Entry for the current process */
   memcpy( pcb[currentProcess].ctx, ctx, sizeof( ctx_t ) );
   int highestPriorityProcess = nextProcess(pcb, maxProcesses);
   /* Copies content of next highest priority process as the current execution context */
   memcpy( ctx, pcb[highestPriorityProcess].ctx, sizeof( ctx_t ) );
   //No idea why this is useful || relevant to keep track of
   return highestPriorityProcess;
}

//TODO : Set the currentProcess global variable in hilevel.c







//
