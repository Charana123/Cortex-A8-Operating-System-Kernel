#include "IPC.h"

//Semaphore pair functions
extern void sem_post();
extern void sem_wait();

/*
  Sets register r0 to the value of the semaphore counter of the pipe

  @param sem_counter - value of the semaphore counter
*/
static void setr0(int sem_counter){
  int *temp = &sem_counter;
  asm ( "mov r0, %0 \n" // assign r0 = pointer
      :
      : "r" (temp)
      : "r0"  );
}

/*
  We grab the lock, Return if we can read from the shared memory
  @param buffer - Reference to the buffer in question
  @param id - ID of the process that is requesting to check the buffer (either source or destination)
  @return - | If data exists read from the opposite process = true
            | otherwise = false
*/
static int checkRead(buffer_t *buffer, int id){
    setr0(buffer -> sem_counter);
    sem_post();
    if(buffer -> sourcePID == id){ bool temp = buffer -> written2; sem_wait(); return temp; }
    else { bool temp = buffer -> written1; sem_wait(); return temp; }
}

/*
  We grab the lock, Return if we can write from the shared memory
  @param buffer - Reference to the buffer in question
  @param id - ID of the process that is requesting to check the buffer (either source or destination)
  @return - | If data exists written from the opposite process = true
            | otherwise = false
*/
static int checkWrite(buffer_t *buffer, int id){
    setr0(buffer -> sem_counter);
    sem_post();
    if(buffer -> written1 || buffer -> written2) { sem_wait(); return 0; }
    else { sem_wait(); return 1; }
}

/*
  We grab the lock, Write to the buffer of the shared memory
  @param buffer - Reference to the buffer in question
  @param id - ID of the process that is requesting to write to the buffer (either source or destination)
  @param data - Data to be written to the buffer
*/
static void lowwriteBuffer(buffer_t *buffer, int id, int data){
    setr0(buffer -> sem_counter);
    sem_post();
    buffer -> data = data;
    if(buffer -> sourcePID == id){ buffer -> written1 = 1; }
    else { buffer -> written2 = 1; }
    sem_wait();
}

/*
  We grab the lock, Read to the buffer of the shared memory
  @param buffer - Reference to the buffer in question
  @param id - ID of the process that is requesting to read to the buffer (either source or destination)
*/
static int lowreadBuffer(buffer_t *buffer, int id){
    setr0(buffer -> sem_counter);
    sem_post();
    if(buffer -> sourcePID == id){ buffer -> written2 = 0; }
    else { buffer -> written1 = 0; }
    int data = buffer -> data;
    sem_wait();
    return data;
}


//High level functions that read
int readBuffer(buffer_t *buffer, int id){
  int data;
  while(1){
    if(checkRead(buffer,id) == 1){
        data = lowreadBuffer(buffer,id);
        break;
    }
    //printString("Reading"); printInt(id); printString("\n");
    sleep(1);
  }
  return data;
}

void writeBuffer(buffer_t *buffer, int id, int data){
  while(1){
    if(checkWrite(buffer, id) == 1){
      lowwriteBuffer(buffer,id,data);
      break;
    }
    //printString("Writing"); printInt(id); printString("\n");
    sleep(1);
  }
}
