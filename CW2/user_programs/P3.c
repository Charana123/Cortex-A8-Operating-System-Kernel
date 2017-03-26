#include "P3.h"




void main_P3() {

  int r = fork(5);

  while(1){
    for( int i = 0; i < 50; i++ ) {
      if( r == 0 ) { write( STDOUT_FILENO, "P3\n", 2 ); }
      else { write( STDOUT_FILENO, "P3++\n", 2 ); }

      uint32_t lo = 1 <<  8;
      uint32_t hi = 1 << 16;

      for( uint32_t x = lo; x < hi; x++ ) {
        int r = is_prime( x );
      }
    }
  }

  exit( EXIT_SUCCESS );
}
