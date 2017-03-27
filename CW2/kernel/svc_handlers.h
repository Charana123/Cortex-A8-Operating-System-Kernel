#ifndef __SVCHANDLERS_H
#define __SVCHANDLERS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"
#include "PCB.h"
#include "virtualisation.h"
#include "svc_calls.h"



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
  Sets the processes corresponding deallocation flag
  @param buffer - Address of the buffer to deallocate
  @return - | if opposite processes dealloc flag is not set = 0 (Deallocation failed)
            | if both processes dealloc flags are set = 1 (Deallocation success)
*/
int svc_dealloc(buffer_t *buffer, pcb_t *pcb, int procID);

/*
  Removes a process from schedular by removing its existing PCB
  @param processIndex - Index of process to be removed (ProcessID - 1)
*/
void svc_kill(pcb_t *pcb, int processIndex);

#endif
