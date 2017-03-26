#ifndef __PIPE_H
#define __PIPE_H

#include <stdbool.h>


/*
  A shared memory structure that allows 2 way communication.
*/
typedef struct {
  int sourcePID; //Source process of shared memory
  int targetPID; //Target process of shared memory
  int data;      //Shared buffer that is written and read from
  bool written1; //Bool to determine if source process has written to shared memory
  bool written2; //Bool to determine if target process has written to shared memory
  int sem_counter; //Mutex counter to limit access to shared memory buffer
  bool sourceDelloc; //Whether the source wishes to keep the pipe open or close it
  bool targetDealloc; //Whether the target wishes to keep the pipe open or close it
} buffer_t;


#endif
