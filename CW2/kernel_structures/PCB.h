#ifndef __PCB_H
#define __PCB_H

//Imports
#include "buffer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* The kernel source code is made simpler via three type definitions:
 *
 * - a type that captures a Process IDentifier (PID), which is really
 *   just an integer,
 * - a type that captures each component of an execution context (i.e.,
 *   processor state) in a compatible order wrt. the low-level handler
 *   preservation and restoration prologue and epilogue, and
 * - a type that captures a process PCB.
 */

typedef int pid_t;
typedef uint32_t pte_t;

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

/* PCB
      - Process ID
      - Process Context
*/
typedef struct {
  pid_t pid;
  ctx_t ctx;
  int active;
  int basePriority;
  int effectivePriority;
  buffer_t **buffers;
  int nbuffers;
  pte_t T[4096] __attribute__ ((aligned (1 << 14)));
} pcb_t;

#endif
