#ifndef __SVCCALLS_H
#define __SVCCALLS_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include "buffer.h"



// Define a type that that captures a Process IDentifier (PID).

typedef int pid_t;

/* The definitions below capture symbolic constants within these classes:
 *
 * 1. system call identifiers (i.e., the constant used by a system call
 *    to specify which action the kernel should take),
 * 2. signal identifiers (as used by the kill system call),
 * 3. status codes for exit,
 * 4. standard file descriptors (e.g., for read and write system calls),
 * 5. platform-specific constants, which may need calibration (wrt. the
 *    underlying hardware QEMU is executed on).
 *
 * They don't *precisely* match the standard C library, but are intended
 * to act as a limited model of similar concepts.
 */

#define SYS_YIELD      ( 0x00 )
#define SYS_WRITE      ( 0x01 )
#define SYS_READ       ( 0x02 )
#define SYS_FORK       ( 0x03 )
#define SYS_EXIT       ( 0x04 )
#define SYS_EXEC       ( 0x05 )
#define SYS_KILL       ( 0x06 )

//Shared Memory SVC calls
#define SYS_ALLOC      ( 0x07 )
#define SYS_DEALLOC    ( 0x08 )
#define SYS_GETID      ( 0x09 )

#define SIG_TERM       ( 0x00 )
#define SIG_QUIT       ( 0x01 )

//#define EXIT_SUCCESS  ( 0 )
//#define EXIT_FAILURE  ( 1 )

#define  STDIN_FILENO ( 0 )
#define STDOUT_FILENO ( 1 )
#define STDERR_FILENO ( 2 )

/*
  Cooperatively yields progression of current process. Schedules the next process with respect to priority.
*/
extern void yield();
/*
  Write to standard output via UART instance.

  @param fd - File descriptor, ignored, always written to UART instance.
  @param x  - Pointer to string to write into file.
  @param n  - Characters that exist in the string to be written.
  @return   - Characters that have been written to the file.
*/
extern int write( int fd, const void* x, size_t n );

extern int  read( int fd, void* x, size_t n );
/*
   Forks a child process.
   @param basePriority - Base priority of the child
   @return - | Parent = Id of the child)
             | Child  = 0
*/
extern int  fork( int basePriority );
/*
  Graceful termination of process
  @param x - exitStatus, either EXIT_SUCCESS or EXIT_FAILURE, we ignore this and just terminate the process.
*/
extern void exit( int x );
/*
  For the child process, Replaces parents image with that of the childs image (Instructions, usually loaded from memory).
  Trivially, We set the programs pc to the entry point of the child process image (main function)

  @param addr - Address of the entry point (main function) to the child process image
*/
extern void exec( const void* x );
/*
  Kills a process.
  @param pid - Id of process to be killed.
  @param s - Ignored.
*/
extern int  kill( pid_t pid, int x );
/*
  Returns the address of a buffer if one exists. Allocated a buffer between processes if it doesn't exist.

  @param targetPID - target process of the buffer
  @return - | if target process doesn't exist = NULL
            | if a buffer between source and target process already exists = address of existing buffer
            | if a buffer doesn't exist source and target processes, creates ones = address of created buffer
*/
extern buffer_t *alloc(int targetPID);
/*
Deallocated given buffer if it hasn't already already been deallocated. Doesn't deallocate the buffer if
input still exists written to it.
@param buffer - Address of the buffer to deallocate
@return - | if input still exists written in the buffer = 0
          | if buffer doesn't exit i.e. has been deallocated previously = 2
          | if buffer gets deallocated = 1
*/
extern int dealloc(buffer_t *buffer);
/*
  Returns the id of the caller process.
  @return - Id of current process.
*/
extern int getid();

#endif
