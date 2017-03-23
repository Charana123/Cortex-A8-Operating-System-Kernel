#include "libc.h"
#include <sys/types.h>
#include <errno.h>


int is_prime( uint32_t x ) {
  if ( !( x & 1 ) || ( x < 2 ) ) {
    return ( x == 2 );
  }
  for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
    if( !( x % d ) ) { return 0; }
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

//Semaphore pair functions
extern void sem_post();
extern void sem_wait();

//Set r0 to semaphore counter
void setr0(int counter){
  int *pointer = &counter;
  asm ( "mov r0, %0 \n" // assign r0 = pointer
      :
      : "r" (pointer)
      : "r0"  );
}

//We grab the lock, check the state of the shared memory, return
int checkRead(buffer_t *buffer, int id){
    setr0(buffer -> sem_counter);
    sem_post();
    if(buffer -> sourcePID == id){ bool temp = buffer -> written2; sem_wait(); return temp; }
    else { bool temp = buffer -> written1; sem_wait(); return temp; }
}

int checkWrite(buffer_t *buffer, int id){
    setr0(buffer -> sem_counter);
    sem_post();
    if(buffer -> written1 || buffer -> written2) { sem_wait(); return 0; }
    else { sem_wait(); return 1; }
}

//We grab the lock, manipulate (read or write) the shared memory, return
void lowwriteBuffer(buffer_t *buffer, int id, int data){
    setr0(buffer -> sem_counter);
    sem_post();
    buffer -> data = data;
    if(buffer -> sourcePID == id){ buffer -> written1 = 1; }
    else { buffer -> written2 = 1; }
    sem_wait();
}

int lowreadBuffer(buffer_t *buffer, int id){
    setr0(buffer -> sem_counter);
    sem_post();
    if(buffer -> sourcePID == id){ buffer -> written2 = 0; }
    else { buffer -> written1 = 0; }
    int data = buffer -> data;
    sem_wait();
    return data;
}

int readBuffer(buffer_t *buffer, int id){
  int data;
  while(1){
    if(checkRead(buffer,id) == 1){
        data = lowreadBuffer(buffer,id);
        break;
    }
    sleep(1);
  }
  return data;
}

void writeBuffer(buffer_t *buffer, int id, int data){
  while(1){
    if(checkWrite(buffer, id) == 1){
      lowwriteBuffer(buffer,id,data);
      break;
    }
    sleep(1);
  }
}

void sleep(int freq){
  for(int i = 0; i < freq *  16000000; i++) {}
}

//SVC CALLS
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

buffer_t *alloc(int targetPID) {
  buffer_t *r;
  asm volatile( "mov r0, %2 \n" // assign r0 = targetPID
                "svc %1     \n" // make system call SYS_ALLOC
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_ALLOC), "r" (targetPID)
              : "r0" );
  return r;
}

int dealloc(buffer_t *buffer){
  int r;
  asm volatile( "mov r0, %2 \n" // assign r0 = buffer
                "svc %1     \n" // make system call SYS_DEALLOC
                "mov %0, r0 \n" // assign r = r0
              : "=r" (r)
              : "I" (SYS_DEALLOC), "r" (buffer)
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
