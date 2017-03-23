#include "hilevel.h"

//Globals
pcb_t pcb[ 100 ];
int currentProcess, maxProcesses;

// Address to a program's main() function entry point to the program main
// Address to top of program's allocated stack space
extern void     main_console();
extern uint32_t tos_console;

/*
  Called on execution to intialize the program for execution.
*/
void hilevel_handler_rst(ctx_t* ctx) {

  //Global variable intializations
  memcpy(ctx, &(pcb[0].ctx), sizeof( ctx_t ) );
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
  pcb[ 0 ].buffers = NULL;
  pcb[ 0 ].nbuffers = 0;

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
    /*
      Yield progression of current process. Schedule the next process with respect to priority.
    */
    currentProcess = priorityScheduler(ctx, pcb, currentProcess, maxProcesses);
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

    printString(x);

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

    int index = svc_fork(basePriority, ctx, pcb, maxProcesses);
    maxProcesses++;

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
      Returns the address of a buffer if one exists. Allocated a buffer between processes if it doesn't exist.

      @param targetPID - target process of the buffer
      @return - | if target process doesn't exist = NULL
                | if a buffer between source and target process already exists = address of existing buffer
                | if a buffer doesn't exist source and target processes, creates ones = address of created buffer
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
      while(index != pcb[currentProcess].nbuffers){
        if( (pcb[currentProcess].buffers[index] -> targetPID) == (currentProcess + 1) ){
          found = true;
          break;
        }
        index++;
      }
      //1)Returns existing buffer if it exists
      if(found == true){
        ctx->gpr[0] = (uint32_t) (pcb[currentProcess].buffers[index]);
      }

      //2) Allocates a buffer in target process PCB
      if(found == false){
        pcb[targetPID - 1].nbuffers++;
        pcb[targetPID - 1].buffers = realloc(pcb[targetPID - 1].buffers, pcb[targetPID - 1].nbuffers * sizeof(buffer_t*));
        index = pcb[targetPID - 1].nbuffers - 1;

        pcb[targetPID - 1].buffers[index] = calloc(1,sizeof(buffer_t));
        pcb[targetPID - 1].buffers[index] -> sourcePID = currentProcess + 1;
        pcb[targetPID - 1].buffers[index] -> targetPID = targetPID;
        pcb[targetPID - 1].buffers[index] -> written1 = 0;
        pcb[targetPID - 1].buffers[index] -> written2 = 0;
        pcb[targetPID - 1].buffers[index] -> sem_counter = 1;

        ctx->gpr[0] = (uint32_t) (pcb[targetPID - 1].buffers[index]);
      }
    }

    break;
  }

  case 0x08:{ // 0x08 => dealloc(buffer_t *buffer)
    /*
    Deallocated given buffer if it hasn't already already been deallocated. Doesn't deallocate the buffer if
    input still exists written to it.
    @param buffer - Address of the buffer to deallocate
    @return - | if input still exists written in the buffer = 0
              | if buffer doesn't exit i.e. has been deallocated previously = 2
              | if buffer gets deallocated = 1
    */
      buffer_t *buffer = (buffer_t*) (ctx ->gpr[0]);

      //Returns 0 if buffer has data still written in it
      if(buffer -> written1 || buffer -> written2 ){
        ctx->gpr[0] = 0;
      }

      else {
        //Looks in target process for buffer
        int index = 0;
        while(index != pcb[buffer->targetPID - 1].nbuffers){
          if(pcb[buffer->targetPID - 1].buffers[index] == buffer) { break; }
          index++;
        }
        //Returns 2 if buffer didn't exist (previously deallocated)
        if(index == pcb[buffer->targetPID - 1].nbuffers){
          ctx->gpr[0] = 2;
        }
        //Returns 1 if buffer does exist and got deallocated
        else{
          //Free the buffer_t structure
          free(pcb[buffer->targetPID - 1].buffers[index]);
          //Swap buffer_t pointer index until it gets to the end of list
          for(int i = index; i < pcb[buffer->targetPID - 1].nbuffers - 1; i++ ){
            buffer_t *temp = pcb[buffer->targetPID - 1].buffers[index];
            pcb[buffer->targetPID - 1].buffers[index] = pcb[buffer->targetPID - 1].buffers[index + 1];
            pcb[buffer->targetPID - 1].buffers[index + 1] = temp;
          }
          pcb[buffer->targetPID - 1].nbuffers--;
          //Realloc size of the buffers structure (reduce size)
          pcb[buffer->targetPID - 1].buffers = realloc(pcb[buffer->targetPID - 1].buffers, pcb[buffer->targetPID - 1].nbuffers * sizeof(buffer_t*));
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
