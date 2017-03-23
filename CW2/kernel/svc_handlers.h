#ifndef __SVCHANDLERS_H
#define __SVCHANDLERS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "buffer.h"
#include "PCB.h"



/*
  Forks a child process.
  @param basePriority - Base priority of the child process.
  @return - Index of the child process
*/
int svc_fork(int basePriority, ctx_t* ctx, pcb_t *pcb, int maxProcesses);

#endif
