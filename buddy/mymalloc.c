#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include "llist.h"

char isFirstRun = '1';

char _heap[MEMSIZE] = {0}; //Simulation of the heap, each char represents a byte in mem

//* node->key and node->length are in bytes
TNode* _memlist = NULL; //Keeps track of allocated mem blks

TNode* buddySystemArr[MAX_ORDER] = {0}; //Keeps track of free mem blks, {0} to init all elements to NULL
//*/

// Do not change this. Used by the test harness.
// You may however use this function in your code if necessary.
long get_index(void *ptr) {
    if(ptr == NULL)
        return -1;
    else
        return (long) ((char *) ptr - &_heap[0]);
}

long get_size(void *ptr) { //Returns size of allocated mem blk in bytes
    if(ptr == NULL){
        return -1;
    }

    //Find in _memlist to ensure it is valid

    long startIndex = get_index(ptr);
    long index = startIndex;

    while(index < MEMSIZE && _heap[index] == '1'){
        ++index;
    }

    if(index == startIndex){ //Size of 0 means ptr is not valid starting address of an allocated mem blk
        return -1;
    }

    return index - startIndex;
}

void print_memlist() {
    if(isFirstRun == '1'){ //Avoid printing uninitialized stuff
        return;
    }

    unsigned int memBlkSize; //In bytes
    TNode* node;

    printf("LinkedList:");

    node = _memlist;

    while(node != NULL){
        printf(" %s, %u, %zu ->",
            node->isTaken == '1' ? "ALLOCATED" : "FREE",
            node->key >> 10,
            node->length >> 10
        );

        node = node->next;
    }





    printf("\n\nBuddySystem:\n");

    for(int i = MAX_ORDER - 1; i >= 0; --i){
        memBlkSize = 1 << (i + 10); //+ 10 for conversion from KiB to bytes

        printf("Block size %u KiB:", memBlkSize >> 10);

        if(buddySystemArr[i] == NULL){ //If all (memBlkSize >> 10) KiB blks are allocated             
            //for(unsigned int startingAddress = 0u; startingAddress < MEMSIZE; startingAddress += memBlkSize){
            //    printf(" %s, %u, %u ->",
            //        "ALLOCATED",
            //        startingAddress >> 10, //Equals to node->key >> 10
            //        memBlkSize >> 10
            //    );
            //}
        } else{
            node = buddySystemArr[i];

            while(node != NULL){
                printf(" %s, %u, %u ->",
                    "FREE", //Always free
                    node->key >> 10,
                    memBlkSize >> 10
                );

                node = node->next;
            }
        }

        puts("");
    }
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    if(size > MEMSIZE){ //If mem blk to alloc cannot fit entire mem
        return NULL;
    }

    if(isFirstRun == '1'){ //Lazy Init (not the most efficient but oh wells)
        TNode* node = make_node(0, NULL);

        node->key = 0; //No need to set isTaken (always free) and length (always 2^(index to buddySystemArr))

        insert_node(buddySystemArr + MAX_ORDER - 1, node, ASCENDING);

        node = make_node(0, NULL);

        node->isTaken = '0';
        node->key = 0;
        node->length = MEMSIZE;

        insert_node(&_memlist, node, ASCENDING);

        isFirstRun = '0';
    }

    int powOfGequalNearestPowOfTwo = -1; //Init with invalid val

    while(size > (1 << (++powOfGequalNearestPowOfTwo + 10))){ //Do nth in loop, + 10 for conversion from KiB to bytes
    }

tryToAlloc:
    if(buddySystemArr[powOfGequalNearestPowOfTwo] != NULL){ //If free mem blk exists
        unsigned int savedStartIndex = buddySystemArr[powOfGequalNearestPowOfTwo]->key; //In bytes

        delete_node(
            buddySystemArr + powOfGequalNearestPowOfTwo,
            buddySystemArr[powOfGequalNearestPowOfTwo]
        ); //Handles all linking of nodes

        TNode* node = find_node(_memlist, savedStartIndex);

        if(node == NULL){
            node = make_node(0, NULL);

            node->isTaken = '1';
            node->key = savedStartIndex;
            node->length = size;

            insert_node(&_memlist, node, ASCENDING);
        } else{
            node->isTaken = '1';
            //node->key = savedStartIndex; //Alr the case
            node->length = size;
        }

        //* Populate heap with actual units allocated
        for(unsigned int i = savedStartIndex; i < savedStartIndex + size; ++i){
            _heap[i] = '1';
        }
        //*/

        return _heap + savedStartIndex;
    } else{
        char canAlloc = '0';
        TNode* node;

        for(int i = powOfGequalNearestPowOfTwo + 1; i < MAX_ORDER; ++i){
            if(buddySystemArr[i] != NULL){ //If free mem blk exists
                unsigned int savedKey = buddySystemArr[i]->key; //Declare here also can since if statement is ran once

                delete_node(
                    buddySystemArr + i,
                    buddySystemArr[i]
                ); //Handles all linking of nodes

                for(int j = i - 1; j >= powOfGequalNearestPowOfTwo; --j){
                    node = make_node(0, NULL);

                    node->key = savedKey + (1 << (j + 10)); //No need to set..., + 10 for...

                    insert_node(buddySystemArr + j, node, ASCENDING);

                    //* Create another free buddy which will be removed when mem blk is allocated
                    if(j == powOfGequalNearestPowOfTwo){
                        node = make_node(0, NULL);

                        node->key = savedKey;

                        insert_node(buddySystemArr + j, node, ASCENDING);
                    }
                    //*/
                }

                canAlloc = '1';

                break;
            }
        }

        if(canAlloc == '0'){ //If not enuf space in mem to alloc mem blk
            return NULL;
        }

        goto tryToAlloc;
    }
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
}

