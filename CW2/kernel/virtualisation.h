#ifndef __VIRTUALISATION_H
#define __VIRTUALISATION_H

#include <stdbool.h>
#include "MMU.h"
#include "PCB.h"
#include "libc.h"

/*
  Intialized Page Frame allocation table
  Mapped : Page 0 - 699 (Interrupt vector table), Page 700 (Kernel & Program image), Page 701, 702(Kernel IRQ & SVC Stack)
  Unmapped : Page 703 - 4095
*/
void initAllocationTable();

/*
  Initializes a Page Table for a new user process
  @param processEntry - Index in the PCB of process to intialize pagetable
*/
void initPageTable(pcb_t *pcb, int processEntry);
/*
  Frees a given page frame to be allocated later. Called to kill or exit a process.
  @param pageframeIndex - The page
*/
void freePageFrame(int pageframeIndex);
/*
  Removes a process from schedular by removing its existing PCB
  @param processIndex - Index of process to be removed (ProcessID - 1)
*/
void svc_kill(pcb_t *pcb, int processIndex);

#endif
