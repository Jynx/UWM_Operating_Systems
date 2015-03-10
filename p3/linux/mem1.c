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
    if (size != 16) {
        return NULL;
    }
 
    if (size + used_memory > available_memory) {
        return NULL;
    }
 
    void* allocated_memory = NULL;
    unsigned char* ptr = header;
    unsigned long long flag;
 
    while(1) {
        unsigned long long* flagPtr = (unsigned long long*)ptr;
        flag = *flagPtr;
        if(flag == 0xFFFFFFFFFFFFFFFF) {
            ptr = ptr + (64*16) + sizeof(unsigned long long);
            continue;
        }
        long long bit = 0;
        long long x;
        for (x = 0; x < 64; ++x) {
            bit = 1ULL << x;
            if ((flag & bit) == 0) {
                bit = x;
                break;
            }
        }
        long long startingByte = ptr - header;
        startingByte += bit * 16 + sizeof(unsigned long long);
        if (startingByte + size > available_memory) {
            return NULL;
        }
        flag = flag | (1ULL << bit);
        *flagPtr = flag;
        allocated_memory = ptr + bit * 16 + sizeof(unsigned long long);
        used_memory += size;
        return allocated_memory;
    }  
}
 
int Mem_Free(void* ptr) {
    if (ptr == NULL) {
        return -1;
    }
 
    unsigned char* current = header;
    unsigned long long flag = 0;
    unsigned long long* flagPtr;
 
    while (1) {
        if (current - header > available_memory) {
            return -1;
        }
        flagPtr = (unsigned long long*)current;
        flag = *flagPtr;
        current += sizeof(unsigned long long);
        if (flag == 0) {
            current = current + (64*16);
            continue;
        }
        long long x;
        for(x = 0; x < 64; ++x) {
            if (current - header > available_memory) {
                return -1;
            }
            if (current == ptr) {
                // Found our target, mark the bit as 0
                flag = flag & ~(1ULL << x);
                *flagPtr = flag;
                return 0;
            }
            current = current + 16;
        }
        if (current - header > available_memory) {
            return -1;
        }
    }
    return 0;
}
//should not include header size.
/*int Mem_Available() {
  int mem_size;
  int free_size;
  block_node* current = NULL;

  current = head_of_list;
  while(current != NULL) {  
    mem_size = current->size_status;
    if(!mem_size & 1) { // &1 = busy block.
      free_size += mem_size;
    }
    current = current->next;
  }
  //fprintf(stdout, "total available memory is: %d\n", free_size);
  return free_size;*/
