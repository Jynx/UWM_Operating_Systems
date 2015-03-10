Our first assignment was a basic C program.

In the linux directory, I created a simple program that reads in a file of N integers 
and used those numbers to prove the accuracy of the birthday paradox theory.

Our Xv6 assignment was designed to help us familiarize ourself with the kernel. We were required to implement a new system call that required a small number of additions in order to facilitate the connection between userspace and the kernel.

In the Xv6 directory, I added all of the necessary "linking" in xv6 in order to create a new addnum system call on both the user and kernel side. I created a global
variable in kernel/defs.h to use as a counter, and used argint to obtain an
argument in my addnum function, that would increment the counter by the
argument amount. 

I also created a unit test that does a couple of basic tests to make sure that
the functionality of the addnum system call is correct. 

