#include "mymalloc.h"

#include "bitmap.h"
#include "llist.h"

#include <stdio.h>
#include <stdlib.h>

char _heap[MEMSIZE] = {0};

static TNode* llist = NULL;

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
    print_map(_heap, MEMSIZE);
}

// Allocates size bytes of memory and returns a pointer
// to the first bit.
void *mymalloc(size_t size) {
    long startBitIndex = search_map(_heap, MEMSIZE, size);
    
    if(startBitIndex == -1){ //Cannot find free blk of mem large enuf to allocate due to external mem fragmentation ("no suitable memory is found")
        return NULL;
    }

    TData* data = (TData*)malloc(sizeof(TData));
    data->len = size;

    TNode* node = find_node(llist, startBitIndex);
    
    if(node == NULL){ //Cannot find node with startBitIndex as key
        node = make_node(startBitIndex, data);
        insert_node(&llist, node, ASCENDING);
    } else{
        node->pdata->len = size; //Update data in existing node
    }

    allocate_map(_heap, startBitIndex, size);

    return _heap + startBitIndex;
}

// Frees memory pointed to by ptr.
void myfree(void *ptr) {
    if(ptr == NULL){
        return; //Fail silently
    }

    long startBitIndex = get_index(ptr);

    TNode* node = find_node(llist, (unsigned int)startBitIndex);

    free_map(_heap, startBitIndex, node->pdata->len);
}

