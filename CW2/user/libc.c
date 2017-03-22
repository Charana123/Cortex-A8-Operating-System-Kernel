#include "libc.h"
#include <sys/types.h>
#include <errno.h>


int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }

  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) {
      return 0;
    }
  }

  return 1;
}



void printString(char *pointer){
  int characters = 0;
  while(pointer[characters] != '\0'){
      characters++;
  }
  write(STDOUT_FILENO, pointer, characters);
}

void printInt(int x){
  char buffer[50];
  itoa(x, buffer, 10);
  printString(buffer);
}

//I need to add a counter in each pipe
//I need to create a function that sets r0 to counter
//I need to call sem_post first then sem_wait

extern void sem_post();
extern void sem_wait();

void setr0(int counter){
  int *pointer = &counter;
  asm ( "mov r0, %0 \n" // assign r0 = pointer
      :
      : "r" (pointer)
      : "r0"  );
}

int checkRead(pipe_t *pipe, int id){
    setr0(pipe -> sem_counter);
    sem_post();
    if(pipe -> sourcePID == id){ return pipe -> written2; }
    return pipe -> written1;
    sem_wait();
}

int checkWrite(pipe_t *pipe, int id){
    setr0(pipe -> sem_counter);
    sem_post();
    if(pipe -> written1 || pipe -> written2) { return 0; }
    return 1;
    sem_wait();
}

void lowwritePipe(pipe_t *pipe, int id, int data){
    setr0(pipe -> sem_counter);
    sem_post();
    pipe -> data = data;
    if(pipe -> sourcePID == id){ pipe -> written1 = 1; }
    else { pipe -> written2 = 1; }
    sem_wait();
}

int lowreadPipe(pipe_t *pipe, int id){
    setr0(pipe -> sem_counter);
    sem_post();
    if(pipe -> sourcePID == id){ pipe -> written2 = 0; }
    else { pipe -> written1 = 0; }
    return pipe -> data;
    sem_wait();
}

int readPipe(pipe_t *pipe, int id){
  int data;
  while(1){
    if(checkRead(pipe,id) == 1){
        data = lowreadPipe(pipe,id);
        break;
    }
    sleep(1);
  }
  return data;
}

void writePipe(pipe_t *pipe, int id, int data){
  while(1){
    if(checkWrite(pipe, id) == 1){
      lowwritePipe(pipe,id,data);
      break;
    }
    sleep(1);
  }
}

void sleep(int freq){
  for(int i = 0; i < freq *  16000000; i++) {}
}

// void writePipe(int id, pipe_t *pipe, int data){
//   while( (pipe -> written1 == true) || (pipe -> written2 == true) ){ yield();  }
//   pipe -> data = data;
//   if(id == pipe -> sourcePID){
//       pipe -> written1 = true;
//       while(pipe -> written1 == true){ yield(); }
//   }
//   if(id == pipe -> targetPID){
//       pipe -> written2 = true;
//       while(pipe -> written2 == true){ yield(); }
//   }
// }
//
// //Waits till written, Read froms pipe
// int readPipe(int id, pipe_t *pipe){
//   if(id == pipe -> sourcePID){
//     while(pipe -> written2 == false) { yield();  }
//     pipe -> written2 = false;
//   }
//   if(id == pipe -> targetPID){
//     while(pipe -> written1 == false) { yield();  }
//     pipe -> written1 = false;
//   }
//   int data = pipe -> data;
//   return data;
// }

void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

//First 3 arguments set the parameters by setting the first 3 registers
//We extract these arguments from the registers in the high level svc call
  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r; // We return 'r' which is assigned via ""mov %0, r0"
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int fork(int basePriority) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = basePriority
                "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_FORK), "r" (basePriority)
              : "r0" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

void exec( const void* x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
              :
              : "I" (SYS_EXEC), "r" (x)
              : "r0" );

  return;
}

int kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}

pipe_t *alloc(int targetPID) {
  pipe_t *r;
  asm volatile( "mov r0, %2 \n" // assign r0 = targetPID
                "svc %1     \n" // make system call SYS_ALLOC
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_ALLOC), "r" (targetPID)
              : "r0" );
  return r;
}

int dealloc(pipe_t *pipe){
  int r;
  asm volatile( "mov r0, %2 \n" // assign r0 = pipe
                "svc %1     \n" // make system call SYS_DEALLOC
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_DEALLOC), "r" (pipe)
              : "r0");
  return r;
}

int getid(){
  int r;
  asm volatile( "svc %1     \n" // make system call SYS_GETID
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_GETID)
              : );
  return r;
}








//Heap Dependencies
//--------------------------------------------------------------------------------
/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");

/* Heap limit returned from SYS_HEAPINFO Angel semihost call.  */
uint __heap_limit = 0xcafedead;

int _sbrk (int incr) {
  extern char end asm ("end"); /* Defined by the linker.  */
  static char * heap_end;
  char * prev_heap_end;

  if (heap_end == NULL)
    heap_end = & end;

  prev_heap_end = heap_end;

  if ((heap_end + incr > stack_ptr)
      || (__heap_limit != 0xcafedead && (int) heap_end + incr > __heap_limit))
    {
      /* Some of the libstdc++-v3 tests rely upon detecting
	 out of memory errors, so do not abort here.  */
#if 0
      extern void abort (void);

      _write (1, "_sbrk: Heap and stack collision\n", 32);

      abort ();
#else
      errno = ENOMEM;
      return -1;
#endif
    }

  heap_end += incr;

  return (int) prev_heap_end;
}
//--------------------------------------------------------------------------------
