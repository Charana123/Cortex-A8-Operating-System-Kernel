#include "hilevel.h"

//Globals
pcb_t pcb[ 100 ];
int currentProcess, maxProcesses;

// Address to a program's main() function entry point to the program main
// Address to top of program's allocated stack space
extern void     main_console();
extern void     main_IPC1();
extern uint32_t tos_console;

/*
  Called on execution to intialize the program for execution.
*/
void hilevel_handler_rst(ctx_t* ctx) {

  //Global variable intializations
  memcpy(ctx, &(pcb[0].ctx), sizeof( ctx_t ) );
  maxProcesses = 1;
  currentProcess = 0;
  initAllocationTable(); //Intialize Page Frame allocation table

  //Intialize Console Program
  //memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].basePriority = 1;
  pcb[ 0 ].effectivePriority = pcb[ 0 ].basePriority;
  pcb[ 0 ].ctx.cpsr = 0x50; // CPSR value = 0x50, Processor is switched into USR mode, with IRQ interrupts enabled
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( 0x70500000 - 8 ); //Top of Page 704
  pcb[ 0 ].active   = 1;
  pcb[ 0 ].buffers = NULL;
  pcb[ 0 ].nbuffers = 0;

  initPageTable(pcb, 0); //Initialize Page Table for Console

  // configure and enable MMU
  mmu_set_ptr0( pcb[0].T );
  mmu_set_dom( 0, 0x3 ); // set domain 0 to 11_{(2)} => manager (i.e., not checked)
  mmu_set_dom( 1, 0x1 ); // set domain 1 to 01_{(2)} => client  (i.e.,     checked)
  mmu_enable();

  //Configure the Timer for intermittent IRQ Hardware Interrupts
  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  //Specify that every existing process other than the console doesn't exist
  for(int i = 1; i<100; i++){
    pcb[i].active = 0;
  }

  //Enable IRQ Interrupts
  int_enable_irq();

  return;
}

void hilevel_handler_pab(ctx_t* ctx) {
  printString("Prefetch Abort\n");
  svc_kill(pcb, currentProcess);
  currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses);
  return;
}

void hilevel_handler_dab(ctx_t* ctx) {
  printString("Data Abort\n");
  svc_kill(pcb, currentProcess);
  currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses);
  return;
}

/*
  High level handler for IRQ (hardware interrupts). Currently only called by Timer.
*/
void hilevel_handler_irq(ctx_t* ctx) {
  //Finds source address of Hardware Interrupt
  uint32_t id = GICC0->IAR;

  //Perform a Context Switch via Scheduler call on intermittent IRQ Interrupts
  if( id == GIC_SOURCE_TIMER0 ) {
    currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses);
    TIMER0->Timer1IntClr = 0x01;
  }

  //Write to the interrupt identifier to signal we're done.
  GICC0->EOIR = id;
  return;
}

/*
  High level hanlder for SVC (software interrupts).
*/
void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {

  switch(id){

  case 0x00 : { // 0x00 => yield()
    currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses);
    break;
  }

  case 0x01 : { // 0x01 => write( fd, x, n )
    int   fd = ( int   )( ctx->gpr[ 0 ] );
    char*  x = ( char* )( ctx->gpr[ 1 ] );
    int    n = ( int   )( ctx->gpr[ 2 ] );
    printString(x);
    ctx->gpr[ 0 ] = n;
    break;
  }

  case 0x03:{ // 0x03 => fork(int basePriority)
    int basePriority = (int) (ctx ->gpr[0]);
    int index = svc_fork(basePriority, ctx, pcb, maxProcesses);
    maxProcesses++;
    ctx->gpr[0]= pcb[ index ].pid; //Register 0 (Return Value) of Parent is the Process ID of Child
    pcb[ index ].ctx.gpr[0] = 0; //Register 0 (Return Value) of Child is 0
    break;
  }

  case 0x04:{ //0x04 => exit(int x)
    int exitStatus = (int) (ctx ->gpr[0]);
    svc_kill(pcb, currentProcess);
    currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses);
  }

  case 0x05:{ // 0x05 => exec(addr)
    void* addr = (void*) (ctx ->gpr[0]);
    //Change the SVC Stack "PC" (Context Structure) to the Function Address of main.
    //Doing SVC Prologue - Virtual "PC" Copied to from SVC Stack to SVC LR and LR is copies to the Actual Program Counter (PC)
    ctx -> pc = (uint32_t) addr;
    break;
  }

  case 0x06:{ // 0x06 => kill(pid,s)
    int pid = (int) (ctx ->gpr[0]);
    int x = (int) (ctx ->gpr[1]);
    svc_kill(pcb, pid-1);
    if(pid - 1 == currentProcess) { currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses); }
    break;
  }


  case 0x07:{ // 0x07 => alloc(targetPID)
    //Gets Target and Source Processes
    int targetPID = (int) (ctx ->gpr[0]);
    buffer_t* buffer = svc_alloc(targetPID, pcb, currentProcess);
    ctx->gpr[0] = (uint32_t) buffer;
    break;
  }

  case 0x08:{ // 0x08 => dealloc(buffer_t *buffer)
    buffer_t *buffer = (buffer_t*) (ctx ->gpr[0]);
    pid_t procID = currentProcess + 1;
    int deallocStatus = svc_dealloc(buffer, pcb, procID);
    ctx->gpr[0] = (uint32_t) deallocStatus;
    break;
  }

  case 0x09:{ // 0x09 => getid()
    int processID = currentProcess + 1;
    ctx ->gpr[0] = processID;
    break;
  }

  default   : { // 0x?? => unknown/unsupported
    int y;
    break;
  }
}
  return;
}
