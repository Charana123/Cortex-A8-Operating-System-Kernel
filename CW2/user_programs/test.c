#include "test.h"


void main_test() {
  //Eat process gets its ID
  int id = getid();

  //Forks off a 16 child process, last id = 18
  if(id < 3){
      pid_t pid = fork( 5 );
      if( 0 == pid ) {
        printString("Child\n");
        exec(&main_test);
      }
  }

  //Allocates buffer with master
  buffer_t *buffer = NULL;
  if(id % 2 == 0) { buffer = alloc(id + 1); }
  else { buffer = alloc(id - 1); }

  if(id % 2 == 0) { writeBuffer(buffer,id,999); }

  else {
    int data = readBuffer(buffer,id);
    printString("Data-");
    printInt(data);
    printString("\n");
  }

  if(id % 2 == 0){
    while(1){
      int r = dealloc(buffer);
      if(r != 0) {
        printString("Deallocation-");
        printInt(r);
        printString("\n");
        break;
      }
    }
  }
  else{
    while(1){
      int r = dealloc(buffer);
      if(r != 0) {
        printString("Deallocation-");
        printInt(r);
        printString("\n");
        break;
      }
    }
  }

  exit( EXIT_SUCCESS );
}
