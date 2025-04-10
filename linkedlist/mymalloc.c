#include <stdio.h>
#include <stdlib.h>

#include "llist.h"
#include "mymalloc.h"

char _heap[MEMSIZE] = {0}; //Each char represents 1 unit allocated (given as 1 byte in brief) in mem (bits do not matter here)

TNode *_memlist = NULL; // To maintain information about length

// Do not change this. Used by the test harness.
// You may however use this function in your code if necessary.
long get_index(void *ptr) {
    if(ptr == NULL)
        return -1;
    else
        return (long) ((char *) ptr - &_heap[0]);
}

void print_memlist(){
    TNode* node = _memlist;

    while(node != NULL){
        printf("Status: %s Start index: %u Length: %zu\n",
            node->isTaken == '1' ? "ALLOCATED" : "FREE",
            node->key,
            node->length
        );

        node = node->next;
    }
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    if(_memlist == NULL){ //If _memlist is empty, Lazy Init
        if(size > MEMSIZE){ //If unit to alloc cannot fit entire mem
            return NULL;
        }

        TNode* node = make_node(0, NULL);

        node->isTaken = '1';
        node->length = size;

        insert_node(&_memlist, node, ASCENDING);

        node = make_node(size, NULL);

        node->isTaken = '0';
        node->length = MEMSIZE - size;

        insert_node(&_memlist, node, ASCENDING);

        //* Populate heap with actual units allocated
        for(size_t i = 0; i < size; ++i){
            _heap[i] = '1';
        }
        //*/

        return _heap; //return _heap + 0;
    }

    TNode* node = _memlist;

    //* Try to find node that represents blk of free mem large enuf for "size"
    while(node != NULL){
        if(node->isTaken == '0' && node->length >= size){
            break;
        }

        node = node->next;
    }
    //*/

    if(node == NULL){ //If cannot find node that...
        //Mem is partitioned by node(s) in _memlist after Lazy Init
        //Shows that unit to alloc cannot fit anywhere in mem due to external mem fragmentation
        return NULL;
    }

    node->isTaken = '1';

    if(node->length == size){
        //* Populate heap with actual units allocated
        for(size_t i = node->key; i < node->key + size; ++i){ //size instead of node->length as slightly more efficient
            _heap[i] = '1';
        }
        //*/

        return _heap + node->key;
    }

    //* if node->length > size (create another node to indicate new smaller free space in mem)
    unsigned int savedKey = node->key;
    size_t savedLength = node->length;

    node->length = size;

    node = make_node(savedKey + size, NULL); //savedKey instead of node->key as slightly more efficient

    node->isTaken = '0';
    node->length = savedLength - size;

    insert_node(&_memlist, node, ASCENDING);

    //* Populate heap with actual units allocated
    for(size_t i = savedKey; i < savedKey + node->length; ++i){
        _heap[i] = '1';
    }
    //*/

    return _heap + savedKey;
    //*/
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
    if(ptr == NULL){
        return; //Fail silently
    }

    long startIndex = get_index(ptr);

    TNode* node = find_node(_memlist, (unsigned int)startIndex);

    if(node == NULL){ //ptr "does not point to a memory region created by mymalloc"
        return; //Fail silently
    }

    node->isTaken = '0';

    //* Merge in next node if it exists and represents free mem
    if(node->next != NULL && node->next->isTaken == '0'){ //Fine if node->next == NULL due to McCarthy evaluation
        node->length += node->next->length;
        
        delete_node(&_memlist, node->next); //Handles all linking of nodes
    }

    //* Merge into prev node if it exists and represents free mem
    if(node->prev != NULL && node->prev->isTaken == '0'){ //Fine if node->prev == NULL due to McCarthy evaluation
        node->prev->length += node->length;

        delete_node(&_memlist, node); //Handles all linking of nodes
    }
    //*/

    //* Depopulate heap with actual units deallocated
    for(size_t i = startIndex; i < node->length; ++i){
        _heap[i] = '0';
    }
    //*/
}

void destroy_memlist(){
    TNode* node = _memlist;
    TNode* savedNextNode; //Just declare as will init in while loop

    while(node != NULL){
        savedNextNode = node->next;

        delete_node(&_memlist, node); //Handles all linking of nodes
        
        node = savedNextNode;
    }
}

