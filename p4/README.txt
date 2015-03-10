Linux-Implementation Details: For this assignment, we were required to modify a small provided web server to be thread safe. I implemented a locking
scheme using condition variables and a producer/consumer model. I used the
main while loop in main() as a quasai producer (used it to populate the buffer
pool) and had a consumer method supported by a get() function call that would
use the file descriptors in the populated buffer. 

XV6-Implementation Details: For our xv6 project, we had to implement multi threading in the xv6 kernel.  I did this by implementing a
number of system calls to breach the userspace/kernel gap, locks, as well as
modifications of existing functions in xv6 to create something that would
create threads as opposed to extra processes (the standard implementation). We had to implement a cloning function that could copy stack data between processes when required. 

Most work can be found in kernel/syscall.c kernel/proc.c kernel/sysproc.c and kernel/proc.h, user/ulib.c . Functions will have my name ass a commented header.
