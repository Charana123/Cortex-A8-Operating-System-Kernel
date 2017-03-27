#include "test.h"


void print(char *string){
  printString(string);
}

void main_test() {
  pid_t pid = getid();
  printString("Process-"); printInt(pid); printString("\n");

  int r0 = fork( 5 );
  if( 0 == r0 ) {
    printString("Forked\n");
    exec(&main_test);
  }
  printString("Trying to Allocate\n");
  buffer_t *buffer = alloc(1);
  printString("AllocatedBuffer-"); printInt(pid); printString("\n");


  //bool found = false;
  while(1){
    //if(found == false) { printString("Ended\n"); found = true; }
  }

  exit( EXIT_SUCCESS );
}












//
