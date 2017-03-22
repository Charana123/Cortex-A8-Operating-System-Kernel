#ifndef __PIPE_H
#define __PIPE_H

typedef struct {
  int sourcePID;
  int targetPID;
  int data;
  bool written1;
  bool written2;
  int sem_counter;
} pipe_t;


#endif
