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
  printString("Philosopher"); printInt(id); printString("\n");
  buffer_t *buffer = alloc(idOfMaster);
  printString("Philosopher Process");
  printInt(id);
  printString(":Allocated Buffer-\n");

  while(1){
    //Asks for forks
    printString("Philosopher Asked forks from Master"); printInt(id); printString("\n");
    writeBuffer(buffer,id,1);
    printString("Philosopher Finished Asking forks from Master"); printInt(id); printString("\n");
    int data = readBuffer(buffer, id);
    if(data == 1){
      printString("Philosopher Process");
      printInt(id);
      printString("-Started Eating\n");
      //Gives back forks
      printString("Philosopher Giving back forks to Master"); printInt(id); printString("\n");
      writeBuffer(buffer,id,0);
      printString("Philosopher Finished Giving back forks to Master"); printInt(id); printString("\n");
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
