#ifndef __LIBC_H
#define __LIBC_H

#include <stdbool.h>
#include <stdlib.h>
#include "PL011.h"

//Prints a Integer via itoa then UART instance
extern void printInt(int x);
//Prints String via UART instance
extern void printString(char *pointer);
//Is a number prime
extern int is_prime( uint32_t x );
//Read from buffer
//Sleep function
extern void sleep(int freq);


#endif
