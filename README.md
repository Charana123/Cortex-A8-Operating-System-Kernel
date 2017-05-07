* Pre-emptive multi-tasking (Part 1)
    * Yes, processes context switch pre-emptively based on a timer in a round-robin manner.
* Dynamic Creation & Termination (Part 2A)
    * How does fork work ?
        * Replicated PCB of parent process into as the Child PCB Entry. Changes associated flags including Process ID & Base Priority.
    * How does exec work ?
        * Changes where the PC points to after the system call returns to the function address (or entry point) of the main function of the associated process.
    * How does kill & exit work ?
        * Kill works by removing the PCB Entry from the Process Table of the Kernel. Exit is a more specific case of where the process killed is the current process.
* Priority Based Scheduler (Part 2B)
    * Why used priority based approach ?
        * IO & CPU Bound processes. Non-priority based schedulers penalise IO Bound processes as they execute for less than the time quantum as they would cooperatively yield execution more often. A dynamic priority based allows priority to be assigned based on current state and type of process.
    * How was it implemented ?
        * Each process given Base and Effective Priority.
        * Effective priority increments if process did not run that turn.
        * Effective priority reset to base priority if the process ran that turn.
        * The process with the highest effective priority is scheduled next.
    * Advantages over non-priority based approached (Round-Robin) ?
        * Advantages over time based priority scheduler
        * General advantages ?
* IPC Mechanism (Part 2C)
    * How was IPC implemented ?
        * Implemented a shared memory.
            * System Calls - SHM_OPEN & SHM_CLOSE (alloc & dealloc respectively)
                * SHM_OPEN - Allocated shared memory (or buffer), Each buffer has an associated mutex. When calls either allocates shared memory or not and returns pointer.
                * SHM_CLOSE - Each program can set this flag in the buffer. When both processes have set this flag the buffer is deallocated. This avoids closing the buffer while there exists pending communications (reads/writes)
            * User Calls - READ & WRITE
                * Read - Grabs lock. Probes buffer if there exists written data. If so, reads data else releases lock.
                * Write - Vice Versa.
            * The shared memory buffer is naively allocated arbitrarily without being written to a specific page and given a shared mapped to each of the communicating programs virtualized space and protected from other programs.
    * How does it prevent mutual exclusion ?
        * The processes synchronize and achieve mutual exclusion by use of a mutex.
        * SEM_wait and SEM_post functions enforce that only one process enters the critical region (and manipulate the contents of the shared memory ie. r/w) at any time.
        * Alternatively this atomicity is achieved by kernel system calls in the case of pipes.
    * How was your Dining Philosophers implemented ?
        * Forks 16 times. Each philosopher opens a shared memory channel to a master philosopher. Master in-turn connects to each philosopher.
        * Each philosopher in turn asks for forks from the Master philosopher. Either 2 forks are given at once and the philosopher start eating. Else the philosopher is asked to wait.
    * How does it prevent starvation ?
        * There is no case that the program can achieve deadlock. The "Wait for" condition for deadlock where a philosopher gets one fork and waits for more is broken. Since each philosopher is either atomically given 2 forks at once or forced to wait.
* MMU (Part 3)
    * Features of MMU ?
        * Virtualization
            * Each process is given its own virtual address space by being assigned its dedicated page table in the MMU.
            * The individual process have a shared mapping for everything up till the Kernel Stacks.
            * Each program Stacks is individually assigned to a different page frame in physical memory. Giving it a dedicated Stack.
        * Protection
            * Page Table Initialization ?
                * Page 0 - 700: Interrupt Table (Program READ/WRITE Access)
                * Page 700: Kernel & Program(s) Image(s) (Program READ ONLY Access)
                * Page 701: SVC Stack (Only Kernel Access)
                * Page 702: IRQ Stack  (Only Kernel Access)
                * Page 703: ABT Stack (Only Kernel Access)
            * The Kernel Program & User Programs given a shared mapping and are Read Only such that they are protected.
            * The Kernel Stacks can only be accessed when in a Kernel Mode.
            * Each Program Stack is protected from each other as no mapping exist to any stack other than their own.
    * Possible Modifications ?
        * Dynamically grow stack (allocate pages) when stack runs out (DAB, data abort)
        * Dedicated Heap (allocate heap)
        * 2 Level Page Table (Page the Page Table)