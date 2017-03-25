#include "svc_handlers.h"


/*
  Creates a PCB (Process Block) entry for a newly created process. Used during a fork system call for creation of child
  process.

  @param ctx - Context, SVC stack pointer (state of gprs) to be copied to the newly created process.
               Context is that of the parent process.
  @param basePriority - Base priority of the newly created process. Given to child by parent process.
  @param index - Index in the PCB Table that the newly created Process Block exists.
*/
void createPCB(ctx_t* ctx, pcb_t *pcb, int basePriority, int nextFreePCB, int maxProcesses){
  //memset( &pcb[ nextFreePCB ], 0, sizeof( pcb_t ) );
  memcpy( &pcb[ nextFreePCB ].ctx, ctx, sizeof( ctx_t ) ); //Copy parent context into child
  initPageTable(pcb, nextFreePCB); //Initialize Page table for child process
  pcb[ nextFreePCB ].ctx.sp   = (uint32_t) (0x70500000 - 8); //Top of Page 704
  pcb[nextFreePCB].pid = nextFreePCB + 1; // Gives the process an id
  pcb[nextFreePCB].active = 1; // This states the program is active
  pcb[nextFreePCB].basePriority = basePriority;
  pcb[nextFreePCB].effectivePriority = pcb[nextFreePCB].basePriority;
  pcb[nextFreePCB].buffers = NULL;
  pcb[nextFreePCB].nbuffers = 0;
}


int svc_fork(int basePriority, ctx_t *ctx, pcb_t *pcb, int maxProcesses){
  //Tries to find an empty PCB(as a result of a killed Process)
  int nextFreePCB; bool found = true;
  for(nextFreePCB = 0; pcb[nextFreePCB].active == 1; nextFreePCB++){
    if(nextFreePCB == maxProcesses) { found = false; break; }
  }
  //If one if found, it is populated with the child process
  if(found == true) { createPCB(ctx, pcb, basePriority, nextFreePCB, maxProcesses); return nextFreePCB; }
  //Else a new PCB is created at the end of the PCB list
  else { createPCB(ctx, pcb, basePriority, maxProcesses, maxProcesses); return maxProcesses; }
}

void svc_kill(pcb_t *pcb, int processIndex){
  pcb[processIndex].active = 0;
  freePageFrame(pcb[processIndex].T[0x704] >> 20); //Unallocate previously allocated stack pageframe
}


buffer_t* svc_alloc(int targetPID, pcb_t *pcb, int currentProcess){
  //returns NULL If the target process doesn't exist
  if(pcb[targetPID - 1].active == 0) { return NULL; }

  //If the target process exists it has 2 options
  else{
    //Looks for existing PIPE in its PCB
    bool allocatedPipeExists = false; int existingPipeIndex = 0;
    while(existingPipeIndex != pcb[currentProcess].nbuffers){
      if( (pcb[currentProcess].buffers[existingPipeIndex] -> targetPID) == (currentProcess + 1) ) { allocatedPipeExists = true; break; }
      existingPipeIndex++;
    }
    //1)Returns existing buffer if it exists
    if(allocatedPipeExists == true){ return pcb[currentProcess].buffers[existingPipeIndex]; }

    //2) Allocates a buffer in target process PCB
    if(allocatedPipeExists == false){
      pcb[targetPID - 1].nbuffers++;
      pcb[targetPID - 1].buffers = realloc(pcb[targetPID - 1].buffers, pcb[targetPID - 1].nbuffers * sizeof(buffer_t*));
      int newPipeIndex = pcb[targetPID - 1].nbuffers - 1;

      pcb[targetPID - 1].buffers[newPipeIndex] = calloc(1,sizeof(buffer_t));
      pcb[targetPID - 1].buffers[newPipeIndex] -> sourcePID = currentProcess + 1;
      pcb[targetPID - 1].buffers[newPipeIndex] -> targetPID = targetPID;
      pcb[targetPID - 1].buffers[newPipeIndex] -> written1 = 0;
      pcb[targetPID - 1].buffers[newPipeIndex] -> written2 = 0;
      pcb[targetPID - 1].buffers[newPipeIndex] -> sem_counter = 1;

      return pcb[targetPID - 1].buffers[newPipeIndex];
    }
  }
}


int svc_dealloc(buffer_t *buffer, pcb_t *pcb){
        //Returns 0 if buffer has data still written in it
        if(buffer -> written1 || buffer -> written2 ){ return 0; }

        //Looks in target process for buffer
        else {
          int buffertoDeallocIndex = 0;
          while(buffertoDeallocIndex != pcb[buffer->targetPID - 1].nbuffers){
            if(pcb[buffer->targetPID - 1].buffers[buffertoDeallocIndex] == buffer) { break; }
            buffertoDeallocIndex++;
          }
          //Returns 2 if buffer didn't exist (previously deallocated)
          if(buffertoDeallocIndex == pcb[buffer->targetPID - 1].nbuffers){ return 2;}

          //Returns 1 if buffer does exist and got deallocated
          else{
            //Free the buffer_t structure
            free(pcb[buffer->targetPID - 1].buffers[buffertoDeallocIndex]);
            //Swap buffer_t pointer index until it gets to the end of list
            for(int i = buffertoDeallocIndex; i < pcb[buffer->targetPID - 1].nbuffers - 1; i++ ){
              buffer_t *temp = pcb[buffer->targetPID - 1].buffers[i];
              pcb[buffer->targetPID - 1].buffers[i] = pcb[buffer->targetPID - 1].buffers[i + 1];
              pcb[buffer->targetPID - 1].buffers[i + 1] = temp;
            }
            pcb[buffer->targetPID - 1].nbuffers--;
            //Realloc size of the buffers structure (reduce size)
            pcb[buffer->targetPID - 1].buffers = realloc(pcb[buffer->targetPID - 1].buffers, pcb[buffer->targetPID - 1].nbuffers * sizeof(buffer_t*));
            return 1;
          }
        }
}




//
