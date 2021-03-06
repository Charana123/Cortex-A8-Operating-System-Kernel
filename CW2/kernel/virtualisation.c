#include "virtualisation.h"


bool pageframe[0xFFF];

/*
  Finds the next free physical page frame
  @return - | if free page frame exists = index of page frame
            | if physical memory has run out (no free page frame) = 0
*/
static uint32_t nextFreePageFrame(){
  for(uint32_t i = 0x700; i < 0xFFF; i++){
    if(pageframe[i] == false) { return i; }
  }
  return 0;
}

void freePageFrame(int pageframeIndex){
  pageframe[pageframeIndex] = false;
  return;
}

void initAllocationTable(){
  for(int i = 0; i <= 0x708; i++){ pageframe[i] = true; }
  for(int i = 0x709; i < 0xFFF; i++){ pageframe[i] = false; }
  return;
}



void initPageTable(pcb_t *pcb, int processEntry){
  //Intialize user process page table

  for(uint32_t i = 0; i <= 0x705; i++){ //Page 0 - 699
    pcb[processEntry].T[i] = ((pte_t) (i) << 20) | 0x00C22; //Interrupt Table - Client Domain | Client Permissions - Full Access
  }                                                         //Page 700 (Kernel & Program Image), Client Domain | Client Permissions - Full Access
  pcb[processEntry].T[0x706] = ((pte_t) (0x701) << 20) | 0x00422; //Page 701 (SVC Stack), Client Domain | Client Permissions - No access
  pcb[processEntry].T[0x707] = ((pte_t) (0x702) << 20) | 0x00422; //Page 702 (IRQ Stack), Client Domain | Client Permissions - No access
  pcb[processEntry].T[0x708] = ((pte_t) (0x703) << 20) | 0x00422; //Page 703 (ABT Stack), Client Domain | Clinet Permissions - No access
  //Finds a free page frame and allocates that as the user's stack page.
  uint32_t stackPageFrame = nextFreePageFrame(pageframe);
  pcb[processEntry].T[0x709] = ((pte_t) (stackPageFrame) << 20) | 0x00C22; //Page 703 (Console Stack), Client Domain | Client Permissions - Full access
  pageframe[stackPageFrame] = true;
  return;
}


















//
