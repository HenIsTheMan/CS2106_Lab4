#include <stdio.h> //Added for size_t

#define MEMSIZE 8         // Size of memory in bytes (was 64)

// Used by the test harness
long get_index(void *ptr);

// Debugging routine
void print_memlist();

void *mymalloc(size_t size);
void myfree(void *);
