#include "master.h"
#define N 3

buffer_t *buffers[N];
int forks = 2;

extern void sem_post();
extern void sem_wait();

void main_master() {
  //Gets its id
  int id = getid();
  printString("Master Process ID");
  printInt(id);
  printString("\n");

  //Allocated buffers with the next 16 processes and holds their references in a list
  //If the target process hasn't been created yet (active = 0) it will repeatedly probe until the target process is created
  for(int i = 1; i <= N; i++){
    while(1){
        buffers[i-1] = alloc(id + i);
        if(buffers[i-1] != NULL){ break; }
    }
  }
  printString("Master - Allocated all buffers\n");

  while(1){
    for(int i=0; i<N; i++){
      //printString("Trying to Read from Philospher"); printInt(i); printString("\n");
        int data = readBuffer(buffers[i],id);
        //printString("Finished Reading from Philospher"); printInt(i); printString("\n");
        //data = 1, The philosopher is asking for a fork
        if(data == 1){
          //If there are enough forks, we signal the philosopher it can have a fork (writes a 1)
          if ( (forks - 2) >= 0){
            forks = forks - 2;
            //printString("Writing 1 to Philospher"); printInt(i); printString("\n");
            writeBuffer(buffers[i],id,1);
            //printString("Finishing Writing 1 to Philospher"); printInt(i); printString("\n");
          }
          else {
            //printString("Writing 0 to Philospher"); printInt(i); printString("\n");
            writeBuffer(buffers[i],id,0);
            //printString("Finished Writing 0 to Philospher"); printInt(i); printString("\n");
          }
        }
        //data = 0, The philospher has finished eating, wants to return his forks
        if(data == 0){
          forks = forks + 2;
        }
    }
  }


  exit( EXIT_SUCCESS );
}





//
