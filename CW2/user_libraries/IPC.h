#ifndef __IPC_H
#define __IPC_H

#include <stdbool.h>
#include <stdint.h>
#include "libc.h"
#include "PCB.h"
#include "buffer.h"

//Read from buffer
extern int readBuffer(buffer_t *buffer, int id);
//Write into Buffer
extern void writeBuffer(buffer_t *buffer, int id, int data);


#endif
