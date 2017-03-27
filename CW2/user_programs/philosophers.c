#include "philosophers.h"

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

  //Allocates buffer with master
  buffer_t *buffer = alloc(idOfMaster);
  printString("Philosopher Process");
  printInt(id);
  printString(":Allocated Buffer-\n");

  while(1){
    //Asks for forks
    writeBuffer(buffer,id,1);
    int data = readBuffer(buffer, id);
    if(data == 1){
      printString("Philosopher Process");
      printInt(id);
      printString("-Started Eating\n");
      writeBuffer(buffer,id,0);
    }
    if(data == 0){
      printString("Philosopher Process");
      printInt(id);
      printString("-Denied Eating\n");
    }
  }


  printString("philosopher - end\n");
  exit( EXIT_SUCCESS );
}
