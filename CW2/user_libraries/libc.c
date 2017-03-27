#include "libc.h"



void printString(char *string){
  int characters;
  for(characters = 0; string[characters] != '\0'; characters++){  }
  write(STDOUT_FILENO, string, characters);
}

void printInt(int x){
  char buffer[50];
  itoa(x, buffer, 10);
  printString(buffer);
}

//Sleep function
//extern void sleep(int freq);

void sleep(int freq){
  for(int i = 0; i < freq *  16000000; i++) {}
}
