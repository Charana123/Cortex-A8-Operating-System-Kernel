#include "hilevel.h"


pcb_t pcb[ 100 ], *current = NULL;
int currentProcess, maxProcesses;

void createPCB(ctx_t* ctx, int basePriority, int index){
  memset( &pcb[ index ], 0, sizeof( pcb_t ) );
  pcb[index].pid = index + 1; // Gives the process an id
  pcb[index].active = 1; // This states the program is active
  pcb[index].basePriority = basePriority;
  pcb[index].effectivePriority = pcb[index].basePriority;
  pcb[index].pipes = NULL;
  pcb[index].npipes = 0;

  //Copy Execution Context of Parent of Child
  memcpy( &pcb[ index ].ctx, ctx, sizeof( ctx_t ) );
  pcb[ index ].ctx.sp   = ( uint32_t ) pcb[index-1].ctx.sp + 0x00001000; //Allocate Stack Space
  ctx->gpr[0]= pcb[ index ].pid; //Register 0 (Return Value) of Parent is the Process ID of Child
  pcb[ index ].ctx.gpr[0] = 0; //Register 0 (Return Value) of Child is 0
  if(index == maxProcesses) { maxProcesses++; }
}

void priorityScheduler(ctx_t* ctx){
  //Process enging was implemented
  //During every IRQ interrupt, every process that did not run that turns effective priority is incremented.
  //The process that did run is reset to its base priority.
  for(int index=0; index<maxProcesses; index++){
    if(index == currentProcess) {
      pcb[index].effectivePriority = pcb[index].basePriority;
      continue;
    }
    pcb[index].effectivePriority = pcb[index].effectivePriority + 1;
  }

  /* Saves curent Execution Context from the SVC Stack (Register State + PC + LR + CSPR)
     and save into PCB Data Structure */
   memcpy( &pcb[currentProcess].ctx, ctx, sizeof( ctx_t ) );

   //Find highest priority process
   for(int index=0; index<maxProcesses; index++){
     if(pcb[index].active == 0) { continue; }
     if(pcb[index].effectivePriority > pcb[currentProcess].effectivePriority){
       currentProcess = index;
     }
   }
   memcpy( ctx, &pcb[currentProcess].ctx, sizeof( ctx_t ) );

   //No idea why this is useful || relevant to keep track of
   current = &pcb[currentProcess];
}

// Address to a program's main() function entry point to the program main
// Address to top of program's allocated stack space
extern void     main_console();
extern uint32_t tos_console;


void hilevel_handler_rst(ctx_t* ctx  ) {

  //Global variable intializations
  current = &pcb[ 0 ];
  memcpy( ctx, &current->ctx, sizeof( ctx_t ) );
  maxProcesses = 1;
  currentProcess = 0;

  //Intialize Console Program
  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].basePriority = 1;
  pcb[ 0 ].effectivePriority = pcb[ 0 ].basePriority;
  pcb[ 0 ].ctx.cpsr = 0x50; // CPSR value = 0x50, Processor is switched into USR mode, with IRQ interrupts enabled
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console  );
  pcb[ 0 ].active   = 1;
  pcb[ 0 ].pipes = NULL;
  pcb[ 0 ].npipes = 0;

  //Configure the Timer for Frequency IRQ Hardware Interrupts
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

void hilevel_handler_irq(ctx_t* ctx) {
  //Finds source address of Hardware Interrupt
  uint32_t id = GICC0->IAR;

  //Perform a Context Switch via Scheduler call on intermittent IRQ Interrupts
  if( id == GIC_SOURCE_TIMER0 ) {
    priorityScheduler(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  //Write to the interrupt identifier to signal we're done.
  GICC0->EOIR = id;
  return;
}

//SVC Software Interrupt Handler
void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {

  switch(id){

  case 0x00 : { // 0x00 => yield()
    priorityScheduler(ctx);
    break;
  }

  case 0x01 : { // 0x01 => write( fd, x, n )
    int   fd = ( int   )( ctx->gpr[ 0 ] );
    char*  x = ( char* )( ctx->gpr[ 1 ] );
    int    n = ( int   )( ctx->gpr[ 2 ] );

    for(int i = 0; i < n; i++ ) {
      PL011_putc( UART0, *x++, true );
    }

    ctx->gpr[ 0 ] = n;
    break;
  }

  case 0x03:{ // 0x03 => fork()
    //Base Priority of is set in the fork system call itself.
    int basePriority = (int) (ctx ->gpr[0]);

    //Tries to find an empty PCB(as a result of a killed Process), if not creates a new PCB
    int fillOldPCB = 0; bool found = true;
    while(pcb[fillOldPCB].active == 1){
      fillOldPCB++;
      if(fillOldPCB >= maxProcesses) { found = false; break; }
    }
    if(found == true) { createPCB(ctx, basePriority, fillOldPCB); }
    else { createPCB(ctx, basePriority, maxProcesses); }
    break;
  }
  case 0x04:{ //0x04 => exit(int x)
    pcb[currentProcess].active = 0;
  }

  case 0x05:{ // 0x05 => exec(addr)
    //Change the SVC Stack "PC" (Context Structure) to the Function Address of "main_P3","main_P4" or "main_P5"
    //Doing SVC Prologue - Virtual "PC" Copied to from SVC Stack to SVC LR and LR is copies to the Actual Program Counter (PC)
    ctx -> pc = (uint32_t) (ctx ->gpr[0]);
    break;
  }

  case 0x06:{ // 0x06 => kill(pid,s)

    int pid = (int) (ctx ->gpr[0]);
    int x = (int) (ctx ->gpr[1]);
    pcb[pid - 1].active = 0; //Sets the active flag of a Process, the Process can no longer be detected by Scheduler
    break;
  }

  case 0x07:{ // 0x07 => alloc(sourcePID,targetPID)
    //Gets Target and Source Processes
    int targetPID = (int) (ctx ->gpr[0]);

    //returns NULL If the target process doesn't exist
    if(pcb[targetPID - 1].active == 0){
        ctx->gpr[0] = (uint32_t) (NULL);
    }

    //If the target process exists it has 2 options
    else{
      //1) Looks for existing PIPE in its PCB
      bool found = false;
      int index = 0;
      while(index != pcb[currentProcess].npipes){
        if( (pcb[currentProcess].pipes[index] -> targetPID) == (currentProcess + 1) ){
          found = true;
          break;
        }
        index++;
      }
      //Returns existing pipe if it exists
      if(found == true){
        ctx->gpr[0] = (uint32_t) (pcb[currentProcess].pipes[index]);
      }

      //2) Allocates a pipe in target process PCB
      if(found == false){
        pcb[targetPID - 1].npipes++;
        pcb[targetPID - 1].pipes = realloc(pcb[targetPID - 1].pipes, pcb[targetPID - 1].npipes * sizeof(pipe_t*));
        index = pcb[targetPID - 1].npipes - 1;

        pcb[targetPID - 1].pipes[index] = calloc(1,sizeof(pipe_t));
        pcb[targetPID - 1].pipes[index] -> sourcePID = currentProcess + 1;
        pcb[targetPID - 1].pipes[index] -> targetPID = targetPID;
        pcb[targetPID - 1].pipes[index] -> written1 = 0;
        pcb[targetPID - 1].pipes[index] -> written2 = 0;
        pcb[targetPID - 1].pipes[index] -> sem_counter = 1;

        ctx->gpr[0] = (uint32_t) (pcb[targetPID - 1].pipes[index]);
      }
    }

    break;
  }

  case 0x08:{ // 0x08 => dealloc(pipe_t *pipe)
      pipe_t *pipe = (pipe_t*) (ctx ->gpr[0]);

      //Returns 0 if pipe has data still written in it
      if(pipe -> written1 || pipe -> written2 ){
        ctx->gpr[0] = 0;
      }

      else {
        //Looks in target process for pipe
        int index = 0;
        while(index != pcb[pipe->targetPID - 1].npipes){
          if(pcb[pipe->targetPID - 1].pipes[index] == pipe) { break; }
          index++;
        }
        //Returns 2 if pipe didn't exist (previously deallocated)
        if(index == pcb[pipe->targetPID - 1].npipes){
          ctx->gpr[0] = 2;
        }
        //Returns 1 if pipe does exist and got deallocated
        else{
          //Free the pipe_t structure
          free(pcb[pipe->targetPID - 1].pipes[index]);
          //Swap pipe_t pointer index until it gets to the end of list
          for(int i = index; i < pcb[pipe->targetPID - 1].npipes - 1; i++ ){
            pipe_t *temp = pcb[pipe->targetPID - 1].pipes[index];
            pcb[pipe->targetPID - 1].pipes[index] = pcb[pipe->targetPID - 1].pipes[index + 1];
            pcb[pipe->targetPID - 1].pipes[index + 1] = temp;
          }
          pcb[pipe->targetPID - 1].npipes--;
          //Realloc size of the pipes structure (reduce size)
          pcb[pipe->targetPID - 1].pipes = realloc(pcb[pipe->targetPID - 1].pipes, pcb[pipe->targetPID - 1].npipes * sizeof(pipe_t*));
          ctx->gpr[0] = 1;
        }
      }
      break;
  }

  case 0x09:{ // 0x09 => getid()
    int processID = currentProcess + 1;
    ctx ->gpr[0] = processID;
    break;
  }


  default   : { // 0x?? => unknown/unsupported
    int x;
    break;
  }
}
  return;
}
