#ifndef __SCHEDULER_H
#define __SCHEDULER_H


#include "PCB.h"

/*
  Deschedules currently running process and Schedules next highest priority process.

  @param pcb - pointer to update global PCB list in hilevel.c
  @param currentProcess - Process that was running before an invocation of the schedular.
  @param ctx - Context of current running process to be saved to its respective PCB.
*/
void priorityScheduler(ctx_t* ctx, pcb_t *pcb, int currentProcess);

#endif
