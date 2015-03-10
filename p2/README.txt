For our C assignment in this project, we were required to build a basic Unix-styled shell. The goal of our assignment was to learn how processes are created, destroyed, and managed.  Our basic set of requirements was: each command creates a new process (for the most part), any program in the path is executable, and implement functionality for commands like exit, cd, pwd, appending, overwrite redirection, and pipes. We also were required to support multiple operators.

For our Xv6 project, we had to implement a round robin style process scheduler, replacing the current scheduler built into Xv6. This included a couple of system calls, adding some variables in the process structure, and designing the scheduler itself.

Most of the work can be found in kernel/sysproc.c and the rand() and scheduler() functions in kernel/proc.c, proc.h
