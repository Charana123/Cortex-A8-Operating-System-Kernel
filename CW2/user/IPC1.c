#include "IPC1.h"

int idOfMaster = 2;

void main_IPC1() {
  //Eat process gets its ID
  int id = getid();

  //Forks off a 16 child process, last id = 18
  if(id < 5){
      pid_t pid = fork( 5 );
      if( 0 == pid ) {
        printString("Child\n");
        exec(&main_IPC1);
      }
  }

  //Allocates pipe with master
  pipe_t *pipe = alloc(idOfMaster);
  printString("Philosopher Process");
  printInt(id);
  printString(":Allocated Pipe-\n");

  while(1){
    //Asks for forks
    writePipe(pipe,id,1);
    int data = readPipe(pipe, id);
    if(data == 1){
      printString("Philosopher Process");
      printInt(id);
      printString("-Started Eating\n");
      //Gives back forks
      writePipe(pipe,id,0);
    }
    if(data == 0){
      printString("Philosopher Process");
      printInt(id);
      printString("-Denied Eating\n");
    }

  }


  printString("philosopher - end");
  exit( EXIT_SUCCESS );
}
