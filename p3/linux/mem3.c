#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

typedef struct block_node{
  struct block_node* next;
  unsigned int size_status;
  /* size of the block is always a multiple of 4 */
  /* ie, last two bits are always zero - can be used to store other information*/
  /* LSB = 0 => free block */
  /* LSB = 1 => allocated/busy block */

  /* For free block, block size = size_status */
  /* For an allocated block, block size = size_status - 1 */

  /* The size of the block stored here is not the real size of the block */
  /* the size stored here = (size of block) - (size of header) */
  /* actual size = size of block + size of header */
  /* when searching for a free block, allocator looks for N + size of header*/
} block_node;

block_node* head_of_list = NULL;

int Mem_Init(int size) {
  void* mem_space_ptr;
  int page_size;
  int padding_size;
  int allocation_size;

  static int has_been_allocated = 0;

  if(has_been_allocated != 0) {
    return -1;
  }
  if (size <= 0) {
    return -1;
  }
  //returns system pagesize
  page_size = getpagesize();

  // calculating the correct paddingsize when rounding to a -size- to a multiple of pagesize
  // *******
  padding_size = size % page_size; 
  padding_size = (page_size - padding_size) % page_size;

  allocation_size = size + padding_size;

  // specifying offset from a file.
  int file;
  if ((file = open("/dev/zero", O_RDWR, 0)) == -1) {
   return -1;
  }
  //file contains offset, 0 = current offset.
  mem_space_ptr = mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file, 0);
  if(mem_space_ptr == MAP_FAILED) {
    has_been_allocated = 0;
    return -1;
  } else {
    has_been_allocated = 1;
  }

  head_of_list = (block_node*)mem_space_ptr; // points to the beginning address of memory range.
  head_of_list->next = NULL;
  /* Remember that the 'size' stored in block size excludes the space for the header */
  head_of_list->size_status = allocation_size - (int)sizeof(block_node);
  //close(file);
  return 0;
}

void* Mem_Alloc(int size) {
  if (size == 0) {
    return NULL;
  }

  int size_check = size % 8;
  //checks to make sure allocation size is a multiple of 8 bytes.
  if (size_check != 0) {
    size += (8 - size_check);
  }

  void* returned_memory = NULL;
  int available_memory = 0;

  //header and next block pointers.
  block_node* itr_node = NULL;
  block_node* next_block_ptr = NULL; 
  char* next_header = NULL;

  itr_node = head_of_list;
  int loop = 1;
  while(loop) {
    if(size <= itr_node->size_status && ((itr_node->size_status) & 1) == 0) {
      //free block found, make sure enough room for next header.
      next_header = (char*)itr_node + (size + (int)sizeof(block_node) * 2); // why *2?
      if(itr_node->next == NULL){
        //use all of the memory, or leave enough room for another header.
        if(size == itr_node->size_status || size <= itr_node->size_status - (int)sizeof(block_node)) {
          break;
        }
      }  
      else if (next_header <= (char*)itr_node->next || size == itr_node->size_status) {
        //didn't go past the next header, so all is good.
        break;
      }
    }
    itr_node = itr_node->next;
    if(itr_node == NULL) {
      return NULL;
    }
  }

  //splitting

  if(itr_node->size_status == size) {
    itr_node->size_status += 1;
    return (char*)itr_node + (int)sizeof(block_node);
  }

  //save existing next ptr and available mem
  next_block_ptr = itr_node->next;
  available_memory = itr_node->size_status;

  //alocate current block and save ptr to return
  itr_node->next = (block_node*)((char*)itr_node + (int)sizeof(block_node) + size);
  itr_node->size_status = size + 1;
  returned_memory = (char*)itr_node + (int)sizeof(block_node);

  //assign the new size in the header for the remaining mem, point to next allocated block
  itr_node = itr_node->next;
  itr_node->next = next_block_ptr;
  itr_node->size_status = available_memory - size - (int)sizeof(block_node);

  return returned_memory;
}

int Mem_Free(void* ptr) {
  if (ptr == NULL) {
    return -1;
  }

  block_node* free_ptr = (block_node*) ptr;
  free_ptr = (block_node*)((char*)ptr - (int)sizeof(block_node));

  if (free_ptr == head_of_list) {
    if(head_of_list->next == NULL) {
      head_of_list->size_status -= 1;
      return 0;
    }
  
    //check to see if next block is ALSO free, and merge
    else if((head_of_list->next->size_status & 1) == 0) {
      head_of_list->size_status = (head_of_list->next->size_status) + (int)sizeof(block_node)
      + (head_of_list->size_status);
      head_of_list->next = head_of_list->next->next;
      head_of_list->size_status = head_of_list->size_status - 1;
      return 0;
    }
    //no consecutive free block found
    else{
      head_of_list->size_status = head_of_list->size_status -1;
      return 0;
    }
  }  

  block_node* itr_node = head_of_list;

  while((free_ptr != (itr_node->next))) {
    itr_node =itr_node->next;
    if(itr_node == NULL) {
      return -1; //ptr not found.
    }
  }
  //found
  itr_node->next->size_status = (itr_node->next->size_status) - 1;

  //check block before free address to merge

  if(((itr_node->size_status) &1) == 0) {
    itr_node->size_status = (itr_node->next->size_status) + (int)sizeof(block_node) +itr_node->size_status;
    itr_node->next = itr_node->next->next;

    if(itr_node->next == NULL) {
      return 0;
    }

    //check for block after free address range to merge

    if((itr_node->next->size_status & 1) == 0) {
      itr_node->size_status = (itr_node->next->size_status) + (int)sizeof(block_node) + itr_node->size_status;
      itr_node->next = itr_node->next->next;
    }
    return 0;
  }

  itr_node = itr_node->next;
  if(itr_node->next == NULL) {
    return 0;
  }

  //case where block after is free but block before is not.

  if((itr_node->next->size_status & 1) == 0) {
    itr_node->size_status = (itr_node->next->size_status) + (int)sizeof(block_node) + itr_node->size_status;
    itr_node->next =itr_node->next->next;
    return 0;
  }
  return 0;
}