all: mem1 mem2 mem3

mem1: mem1.c mem.h
	gcc -c -fpic mem1.c -Wall -Werror
	gcc -shared -o libmem1.so mem1.o
	
mem2: mem2.c mem.h
	gcc -c -fpic mem2.c -Wall -Werror
	gcc -shared -o libmem2.so mem2.o

mem3: mem3.c mem.h
	gcc -c -fpic mem3.c -Wall -Werror 
	gcc -shared -o libmem3.so mem3.o
clean:
	rm -rf mem1.o mem2.o mem3.o libmem1.so libmem2.so libmem3.so