#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// Include functionality relating to the platform.
#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

// Include functionality relating to the kernel.
#include "lolevel.h"
#include "int.h"
#include "libc.h"
#include "buffer.h"
#include "PCB.h"
#include "scheduler.h"
#include "svc_handlers.h"
#include "MMU.h"
#include "virtualisation.h"





#endif
