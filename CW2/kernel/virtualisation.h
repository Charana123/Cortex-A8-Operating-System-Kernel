#ifndef __VIRTUALISATION_H
#define __VIRTUALISATION_H


/*
  Intialized Page Frame allocation table
  Mapped : Page 0 - 699 (Interrupt vector table), Page 700 (Kernel & Program image), Page 701, 702(Kernel IRQ & SVC Stack)
  Unmapped : Page 703 - 4095
*/
void initAllocationTable()

/*
  Initializes a Page Table for a new user process
  @param processEntry - Index in the PCB of process to intialize pagetable
*/
void initPageTable(int processEntry);

#endif
