#include "master.h"
#define N 3

pipe_t *pipes[N];
int forks = 2;

extern void sem_post();
extern void sem_wait();

void main_master() {
  //Gets its id
  int id = getid();
  printString("Master Process ID");
  printInt(id);
  printString("\n");

  //Allocated pipes with the next 16 processes and holds their references in a list
  //If the target process hasn't been created yet (active = 0) it will repeatedly probe until the target process is created
  for(int i = 1; i <= N; i++){
    while(1){
        pipes[i-1] = alloc(id + i);
        if(pipes[i-1] != NULL){ break; }
    }
  }
  printString("Master - Allocated all pipes\n");

  while(1){
    for(int i=0; i<N; i++){
        int data = readPipe(pipes[i],id);
        //data = 1, The philosopher is asking for a fork
        if(data == 1){
          //If there are enough forks, we signal the philosopher it can have a fork (writes a 1)
          if ( (forks - 2) >= 0){
            forks = forks - 2;
            writePipe(pipes[i],id,1);
          }
          else {
            writePipe(pipes[i],id,0);
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







// int a = 5; int b;
// asm ( "mov r0, %1 \n"
//       "mov %0, r0 \n"
//     : "=r" (b)
//     : "r" (a)
//     : "r0"  );

// int a = 5; int b;
// asm ( "mov r0, %0 \n" // assign r0 = basePriority
//     :
//     : "r" (a)
//     : "r0"  );
//
// asm ( "mov %0, r0 \n"
//     : "=r" (b)
//     :
//     : );

// printString("ValueofB -");
// printInt(b);
// printString("-\n");

// int a = 1; int *pointer = &a;
// asm ( "mov r0, %0 \n" // assign r0 = pointer
//     :
//     : "r" (pointer)
//     : "r0"  );
//
// //sem_post();
// sem_wait();
//
// printString("ValueofA -");
// printInt(a);
// printString("-\n");
//
//
