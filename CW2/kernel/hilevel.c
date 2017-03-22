#include "hilevel.h"

<<<<<<< HEAD


=======
>>>>>>> new_branch
pcb_t pcb[ 100 ], *current = NULL;
int currentProcess, maxProcesses;


//Priority Scheduler
/*
  Deschedules currently running process and Schedules next highest priority process.

  @param ctx - Context of current running process to be saved to its respective PCB.
*/
void priorityScheduler(ctx_t* ctx){
  //Process Ageing
  //During every IRQ interrupt, For every process that did not run that turn, its effective priority is incremented.
  //The process that did run that turn is reset to its base priority.
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

/*
  Called on execution to intialize the program for execution.
*/
void hilevel_handler_rst(ctx_t* ctx) {

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

/*
  High level handler for IRQ (hardware interrupts). Currently only called by Timer.
*/
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



//SVC Handlers

/*
  Writes to a file.

  @param fd - File descriptor of file to write to.
  @param x  - Pointer to string to write into file.
  @param n  - Characters that exist in the string to be written.
*/
void svc_write(int fd, char *x, int n){
  for(int i = 0; i < n; i++ ) {
    PL011_putc( UART0, *x++, true );
  }
}

/*
  Creates a PCB (Process Block) entry for a newly created process. Used during a fork system call for creation of child
  process.

  @param ctx - Context, SVC stack pointer (state of gprs) to be copied to the newly created process.
               Context is that of the parent process.
  @param basePriority - Base priority of the newly created process. Given to child by parent process.
  @param index - Index in the PCB Table that the newly created Process Block exists.
*/
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
  if(index == maxProcesses) { maxProcesses++; }
}

/*
  Forks a child process.
  @param basePriority - Base priority of the child process.
  @return - Index of the child process
*/
int svc_fork(int basePriority, ctx_t* ctx){
  //Tries to find an empty PCB(as a result of a killed Process)
  int fillOldPCB = 0; bool found = true;
  while(pcb[fillOldPCB].active == 1){
    fillOldPCB++;
    if(fillOldPCB >= maxProcesses) { found = false; break; }
  }
  //If one if found, it is populated with the child process
  if(found == true) { createPCB(ctx, basePriority, fillOldPCB); return fillOldPCB; }
  //Else a new PCB is created at the end of the PCB list
  else { createPCB(ctx, basePriority, maxProcesses); return (maxProcesses-1); }
}



/*
  High level hanlder for SVC (software interrupts).
*/
void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {

  switch(id){

  case 0x00 : { // 0x00 => yield()
    /*
      Yield progression of current process. Schedule the next process with respect to priority.
    */
    priorityScheduler(ctx);
    break;
  }

  case 0x01 : { // 0x01 => write( fd, x, n )
    /*
      Write to standard output via UART instance.

      @param fd - File descriptor, ignored, always written to UART instance.
      @param x  - Pointer to string to write into file.
      @param n  - Characters that exist in the string to be written.
      @return   - Characters that have been written to the file.
    */
    int   fd = ( int   )( ctx->gpr[ 0 ] );
    char*  x = ( char* )( ctx->gpr[ 1 ] );
    int    n = ( int   )( ctx->gpr[ 2 ] );

    svc_write(fd, x, n);

    ctx->gpr[ 0 ] = n;
    break;
  }

  case 0x03:{ // 0x03 => fork(int basePriority)
    /*
       Forks a child process.
       @param basePriority - Base priority of the child
       @return - Parent (Id of the child) / Child (0)
    */
    int basePriority = (int) (ctx ->gpr[0]);

    int index = svc_fork(basePriority, ctx);

    ctx->gpr[0]= pcb[ index ].pid; //Register 0 (Return Value) of Parent is the Process ID of Child
    pcb[ index ].ctx.gpr[0] = 0; //Register 0 (Return Value) of Child is 0
    break;
  }

  case 0x04:{ //0x04 => exit(int x)
    /*
      Graceful termination of process
      @param x - exitStatus, either EXIT_SUCCESS or EXIT_FAILURE, we ignore this and just terminate the process.
    */
    int exitStatus = (int) (ctx ->gpr[0]);
    pcb[currentProcess].active = 0;
  }

  case 0x05:{ // 0x05 => exec(addr)
    /*
      For the child process, Replaces parents image with that of the childs image (Instructions, usually loaded from memory).
      Trivially, We set the programs pc to the entry point of the child process image (main function)

      @param addr - Address of the entry point (main function) to the child process image
    */
    void* addr = (void*) (ctx ->gpr[0]);
    //Change the SVC Stack "PC" (Context Structure) to the Function Address of main.
    //Doing SVC Prologue - Virtual "PC" Copied to from SVC Stack to SVC LR and LR is copies to the Actual Program Counter (PC)
    ctx -> pc = (uint32_t) addr;
    break;
  }

  case 0x06:{ // 0x06 => kill(pid,s)
    /*
      Kills a process.
      @param pid - Id of process to be killed.
      @param s - Ignored.
    */
    int pid = (int) (ctx ->gpr[0]);
    int x = (int) (ctx ->gpr[1]);
    pcb[pid - 1].active = 0; //Sets the active flag of a Process, the Process can no longer be detected by Scheduler
    break;
  }

  case 0x07:{ // 0x07 => alloc(targetPID)
    /*
      Returns the address of a pipe if one exists. Allocated a pipe between processes if it doesn't exist.

      @param targetPID - target process of the pipe
      @return - | if target process doesn't exist = NULL
                | if a pipe between source and target process already exists = address of existing pipe
                | if a pipe doesn't exist source and target processes, creates ones = address of created pipe
    */
    //Gets Target and Source Processes
    int targetPID = (int) (ctx ->gpr[0]);

    //returns NULL If the target process doesn't exist
    if(pcb[targetPID - 1].active == 0){
        ctx->gpr[0] = (uint32_t) (NULL);
    }

    //If the target process exists it has 2 options
    else{
      //Looks for existing PIPE in its PCB
      bool found = false;
      int index = 0;
      while(index != pcb[currentProcess].npipes){
        if( (pcb[currentProcess].pipes[index] -> targetPID) == (currentProcess + 1) ){
          found = true;
          break;
        }
        index++;
      }
      //1)Returns existing pipe if it exists
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
    /*
    Deallocated given pipe if it hasn't already already been deallocated. Doesn't deallocate the pipe if
    input still exists written to it.
    @param pipe - Address of the pipe to deallocate
    @return - | if input still exists written in the pipe = 0
              | if pipe doesn't exit i.e. has been deallocated previously = 2
              | if pipe gets deallocated = 1
    */
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
    /*
      Returns the id of the caller process.
      @return - Id of current process.
    */
    int processID = currentProcess + 1;
    ctx ->gpr[0] = processID;
    break;
  }


  default   : { // 0x?? => unknown/unsupported
    break;
  }
}
  return;
}
