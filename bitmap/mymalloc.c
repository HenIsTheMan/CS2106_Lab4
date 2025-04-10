#include "mymalloc.h"

#include "bitmap.h"
#include "llist.h"

#include <stdio.h>
#include <stdlib.h>

#define BITMAP_SIZE MEMSIZE / 8

char _heap[MEMSIZE] = {0}; //Each char represents 1 unit allocated (given as 1 byte in brief) in mem (bits do not matter here)

char bitmap[BITMAP_SIZE] = {0}; //Each bit in each char represents 1 unit allocated (given as 1 byte in brief) in mem

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
    print_map(bitmap, BITMAP_SIZE);
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    long startIndex = search_map(bitmap, BITMAP_SIZE, size); //Shared between _heap, bitmap and llist
    
    if(startIndex == -1){ //Cannot find free blk of mem large enuf to allocate due to external mem fragmentation ("no suitable memory is found")
        return NULL;
    }

    TNode* node = find_node(llist, startIndex);
    
    if(node == NULL){ //Cannot find node with startIndex as key
        TData* data = (TData*)malloc(sizeof(TData));
        data->len = size;

        node = make_node(startIndex, data);

        insert_node(&llist, node, ASCENDING);
    } else{
        ///Since free(node->pdata); is called when myfree is called
        ///and only myfree can free node->pdata according to the implementation in testmalloc.c and harness.c
        if(node->pdata == NULL){
            node->pdata = (TData*)malloc(sizeof(TData));
        }

        node->pdata->len = size; //Update data in existing node
    }

    allocate_map(bitmap, startIndex, size); //Populate bitmap with representation of units allocated

    //* Populate heap with actual units allocated
    for(size_t i = startIndex; i < size; ++i){
        _heap[i] = '1';
    }
    //*/

    return _heap + startIndex;
}

// Frees memory pointed to by ptr.
void myfree(void *ptr) {
    if(ptr == NULL){
        return; //Fail silently
    }

    long startIndex = get_index(ptr); //Shared between...

    TNode* node = find_node(llist, (unsigned int)startIndex);

    if(node == NULL){ //ptr "does not point to a memory region created by mymalloc"
        return; //Fail silently
    }

    free_map(bitmap, startIndex, node->pdata->len); //Depopulate bitmap with representation of units deallocated

    //* Depopulate heap with actual units deallocated
    for(size_t i = startIndex; i < node->pdata->len; ++i){
        _heap[i] = '0';
    }
    //*/

    if(node->pdata != NULL){
        free(node->pdata);
        node->pdata = NULL;
    }

    delete_node(&llist, node);
    node = NULL;
}

