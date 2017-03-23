#include "libc.h"



int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }
  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) { return 0; }
  }
  return 1;
}

void printString(char *string){
  for(int i = 0; string[i] != '\0'; i++ ) {
    PL011_putc( UART0, *(string + i), true );
  }
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
