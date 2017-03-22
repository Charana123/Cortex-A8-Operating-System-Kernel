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

  //Allocates pipe with master
  pipe_t *pipe = NULL;
  if(id % 2 == 0) { pipe = alloc(id + 1); }
  else { pipe = alloc(id - 1); }

  if(id % 2 == 0) { writePipe(pipe,id,999); }

  else {
    int data = readPipe(pipe,id);
    printString("Data-");
    printInt(data);
    printString("\n");
  }

  if(id % 2 == 0){
    while(1){
      int r = dealloc(pipe);
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
      int r = dealloc(pipe);
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
