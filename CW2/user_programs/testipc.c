#include "testipc.h"


void main_testipc() {
  //Eat process gets its ID
  int id = getid();

  //Forks off a 16 child process, last id = 18
  if(id < 3){
      pid_t pid = fork( 5 );
      if( 0 == pid ) {
        printString("Child\n");
        exec(&main_testipc);
      }
  }
  printString("Halfway");

  //Allocates buffer with master
  buffer_t *buffer = NULL;
  if(id % 2 == 0) { buffer = alloc(id + 1); }
  else { buffer = alloc(id - 1); printString("Allocated Buffer\n"); }

  if(id % 2 == 0) { writeBuffer(buffer,id,999); }

  else {
    printString("Trying to Read\n");
    int data = readBuffer(buffer,id);
    printString("Data-");
    printInt(data);
    printString("\n");
  }

  if(id % 2 == 0){
      int r = dealloc(buffer);
      printString("Deallocation-");
      printInt(r);
      printString("\n");
  }
  else{
      int r = dealloc(buffer);
      printString("Deallocation-");
      printInt(r);
      printString("\n");
  }

  while(1){

  }

  exit( EXIT_SUCCESS );
}












//
