#ifndef __SVCHANDLERS_H
#define __SVCHANDLERS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"
#include "PCB.h"
#include "hilevel.h"
#include "virtualisation.h"



/*
  Forks a child process.
  @param basePriority - Base priority of the child process.
  @return - Index of the child process
*/
int svc_fork(int basePriority, ctx_t* ctx, pcb_t *pcb, int maxProcesses);

/*
  Returns the address of a buffer if one exists. Allocated a buffer between processes if it doesn't exist.

  @param targetPID - target process of the buffer
  @return - | if target process doesn't exist = NULL
            | if a buffer between source and target process already exists = address of existing buffer
            | if a buffer doesn't exist source and target processes, creates ones = address of created buffer
*/
buffer_t* svc_alloc(int targetPID, pcb_t *pcb, int currentProcess);
/*
Deallocated given buffer if it hasn't already already been deallocated. Doesn't deallocate the buffer if
input still exists written to it.
@param buffer - Address of the buffer to deallocate
@return - | if input still exists written in the buffer = 0
          | if buffer doesn't exit i.e. has been deallocated previously = 2
          | if buffer gets deallocated = 1
*/
int svc_dealloc(buffer_t *buffer, pcb_t *pcb, int procID);
#endif
