#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

char _heap[MEMSIZE] = {0};

// Do not change this. Used by the test harness.
// You may however use this function in your code if necessary.
long get_index(void *ptr) {
    if(ptr == NULL)
        return -1;
    else
        return (long) ((char *) ptr - &_heap[0]);
}

void print_memlist() {
    // Implement this to call print_map from bitmap.c
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    return NULL;
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
}

