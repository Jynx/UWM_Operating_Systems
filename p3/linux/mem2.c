#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

unsigned char* header = NULL;
int available_memory = 0;
int used_memory = 0;
 
int Mem_Init(int size) {
  void* mem_space_ptr;
  int page_size;
  int padding_size;
 
  static int has_been_allocated = 0;
 
  if(has_been_allocated != 0) {
    return -1;
  }
  if (size <= 0) {
    return -1;
  }
  //returns system pagesize
  page_size = getpagesize();
   
  if(size < page_size) {
    available_memory = page_size;
  }
  else {
    // calculating the correct paddingsize when rounding to a -size- to a multiple of pagesize
    // *******
    padding_size = size % page_size; 
    padding_size = (page_size - padding_size) % page_size;
 
    available_memory = size + padding_size;
  }
 
 
  // specifying offset from a file.
  int file;
  if ((file = open("/dev/zero", O_RDWR, 0)) == -1) {
   return -1;
  }
  //file contains offset, 0 = current offset.
  mem_space_ptr = mmap(NULL, available_memory, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0);
  if(mem_space_ptr == MAP_FAILED) {
    has_been_allocated = 0;
    return -1;
  } else {
    has_been_allocated = 1;
  }
  int flag;
  header = (unsigned char*)mem_space_ptr;
  memset(header, 0, available_memory);
  flag = (*(int*)mem_space_ptr); // points to the beginning address of memory range.
 
  close(file);
  return 0;
}

void* Mem_Alloc(int size) {
  if (size != 16 && size != 80 && size != 256) {
    return NULL;
  }
 
  if (size + used_memory > available_memory) {
    return NULL;
  }
 
  //void* allocated_memory = NULL;
  unsigned char* ptr = header;
  unsigned char flag = 0;
 
  while(1) {
    flag = *ptr;
    if (flag == 0) {
        // This slot is free
        if (size == 16)
        {
            flag = 1;
        }
        else if (size == 80)
        {
            flag = 2;
        }
        else
        {
            flag = 3;
        }
        *ptr = flag;
        ptr += sizeof(unsigned char);
        return ptr;
    } else if (flag == 1) {
        ptr += sizeof(unsigned char) + 16;
    } else if (flag == 2) {
        ptr += sizeof(unsigned char) + 80;
    } else if (flag == 3) {
        ptr += sizeof(unsigned char) + 256;
    }
    if (ptr - header + size + 1 > available_memory) {
        return NULL;
    }
  }  
  return NULL;
}
 
int Mem_Free(void* ptr) {
  if (ptr == NULL) {
    return -1;
  }
 
  unsigned char* current = header;
  unsigned char flag = 0;
  
  while (1) {
    if (current - header > available_memory) {
       return -1;
    }
    flag = *current;
    current += sizeof(unsigned char);
    if (current == ptr)
    {
        *current = 0; // freed
        return 0;
    }
    if (flag == 1)
    {
        current += 16;
    }
    else if (flag == 2)
    {
        current += 80;
    }
    else if (flag == 3)
    {
        current += 256;
    }
  }
  return -1;
}