#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include "llist.h"

char isFirstValidRun = '1';

char _heap[MEMSIZE] = {0}; //Simulation of the heap, each char represents a byte in mem

//* node->key and node->length are in bytes
TNode* _memlist = NULL; //Keeps track of all partitions in Buddy System

TNode* buddySystemArr[MAX_ORDER] = {0}; //Keeps track of all free partitions in Buddy System, {0} to init all elements to NULL
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

    unsigned int startIndex = (unsigned int)get_index(ptr);

    //* Check if ptr is a valid starting address of an allocated mem blk
    ///Can check partition as it shares same startIndex with its allocated mem blk
    TNode* node = find_node(_memlist, startIndex);
    
    if(node == NULL){
        return -1;
    }
    //*/

    unsigned int index = startIndex;

    //* index - startIndex <= node->length to prevent counting of '1's that are from another partition (aka from another mem blk)
    ///index - startIndex <= node->length also removes need to check index < MEMSIZE (not possible with correct implementation)
    while(index - startIndex <= node->length && _heap[index] == '1'){
        ++index;
    }
    //*/

    return index - startIndex;
}

void print_memlist() {
    if(isFirstValidRun == '1'){ //Avoid printing uninitialized stuff
        return;
    }

    int checkCount;
    int nextCheckCount = 1; //Only need to check once when sizeToCheck == MEMSIZE
    unsigned int baseStartIndex = 0;
    unsigned int startIndexToCheck;
    size_t sizeLeft = MEMSIZE;
    size_t sizeToCheck = MEMSIZE;
    size_t totalSizeOfSubpartitionsLeft = MEMSIZE;
    TNode* node;

    while(sizeToCheck >= MINIMUM_BLOCK_SIZE){
        printf("Block size %zu KiB:", sizeToCheck >> 10);
        
        if(sizeLeft > 0u){
            startIndexToCheck = baseStartIndex;

            checkCount = nextCheckCount;
            nextCheckCount = 0;

            for(int i = 0; i < checkCount; ++i, startIndexToCheck += sizeToCheck){ //Cannot do startIndexToCheck += node->length as node might be NULL
                node = find_node(_memlist, startIndexToCheck);

                if(node != NULL && node->length == sizeToCheck){ //If found node in _memlist (actual) that is free or fully taken
                    printf(" %s, %u, %zu ->",
                        node->isTaken == '1' ? "ALLOCATED" : "FREE",
                        startIndexToCheck >> 10, //Same as node->key >> 10
                        sizeToCheck >> 10 //Same as node->length >> 10
                    );

                    totalSizeOfSubpartitionsLeft -= sizeToCheck;
                    sizeLeft -= sizeToCheck;
                } else{ //Confirm taken in subpartitions
                    printf(" %s, %u, %zu ->",
                        "ALLOCATED",
                        startIndexToCheck >> 10,
                        sizeToCheck >> 10
                    );

                    if(nextCheckCount == 0){ //We want to lowest valid baseStartIndex
                        baseStartIndex = startIndexToCheck;
                    }

                    nextCheckCount += 2;
                }
            }
        }

        sizeToCheck >>= 1;

        puts("");
    }
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    if(size > MEMSIZE){ //If mem blk to alloc cannot fit entire mem
        return NULL;
    }

    if(isFirstValidRun == '1'){ //Lazy Init (not the most efficient but oh wells)
        TNode* node = make_node(0, NULL);

        node->key = 0; //No need to set isTaken (always free) and length (always 2^(index to buddySystemArr))

        insert_node(buddySystemArr + MAX_ORDER - 1, node, ASCENDING);

        node = make_node(0, NULL);

        node->isTaken = '0';
        node->key = 0;
        node->length = MEMSIZE;

        insert_node(&_memlist, node, ASCENDING);

        isFirstValidRun = '0';
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

        TNode* node = find_node(_memlist, savedStartIndex); //Node definitely exists if _memlist is maintained properly

        node->isTaken = '1';
        //node->key = savedStartIndex; //Alr the case (obvious)
        //node->length = 1 << (powOfGequalNearestPowOfTwo + 10); //+ 10 for...

        //* Populate heap with mem units allocated (can use memset but must #include <string.h>)
        for(unsigned int i = savedStartIndex; i < savedStartIndex + size; ++i){
            _heap[i] = '1';
        }
        //*/

        return _heap + savedStartIndex;
    } else{
        char canAlloc = '0';
        TNode* node;
        unsigned int sharedKey;

        //* Create resulting free partitions from Buddy System splitting alg
        for(int i = powOfGequalNearestPowOfTwo + 1; i < MAX_ORDER; ++i){
            if(buddySystemArr[i] != NULL){ //If free mem blk exists
                unsigned int savedKey = buddySystemArr[i]->key; //Declare here also can since if statement is ran once

                delete_node(
                    buddySystemArr + i,
                    buddySystemArr[i]
                ); //Handles all linking of nodes

                for(int j = i - 1; j >= powOfGequalNearestPowOfTwo; --j){
                    sharedKey = savedKey + (1 << (j + 10)); //+ 10 for...

                    node = make_node(0, NULL);

                    node->key = sharedKey; //No need to set...

                    insert_node(buddySystemArr + j, node, ASCENDING);

                    node = make_node(0, NULL);

                    node->isTaken = '0';
                    node->key = sharedKey;
                    node->length = 1 << (j + 10);

                    insert_node(&_memlist, node, ASCENDING);

                    //* Create free buddy which will be removed when mem blk is allocated
                    if(j == powOfGequalNearestPowOfTwo){
                        node = make_node(0, NULL);

                        node->key = savedKey;

                        insert_node(buddySystemArr + j, node, ASCENDING);

                        node = find_node(_memlist, savedKey); //Node definitely exists if _memlist is maintained properly

                        //node->isTaken = '0'; //Alr the case (not so obvious)
                        //node->key = savedKey; //Alr the case (obvious)
                        node->length = 1 << (j + 10);
                    }
                    //*/
                }

                canAlloc = '1';

                break;
            }
        }
        //*/

        if(canAlloc == '0'){ //If not enuf space in mem to alloc mem blk
            return NULL;
        }

        goto tryToAlloc;
    }
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
    if(ptr == NULL){
        return; //Fail silently
    }

    unsigned int startIndex = (unsigned int)get_index(ptr);

    TNode* linkedListNode = find_node(_memlist, startIndex);

    //ptr "does not point to a memory region created by mymalloc" or mem partition is alr free
    if(linkedListNode == NULL || linkedListNode->isTaken == '0'){
        return; //Fail silently
    }
    //*/

    //* Find pow where 2^pow = linkedListNode->length
    int pow = -1; //Init with invalid val

    while((1 << (++pow + 10)) != linkedListNode->length){ //Do nth in loop, + 10 for conversion from KiB to bytes
    }

CheckBuddy:
    //* Find startIndex of buddy using bitwise XOR to flip the correct bit
    unsigned int bitmask = (unsigned int)linkedListNode->length;

    startIndex = linkedListNode->key; //Need for if linkedListNode gets updated below

    startIndex ^= bitmask; //Calc startIndex of buddy
    //*/

    TNode* buddyNode = find_node(buddySystemArr[pow], startIndex);
    TNode* node;

    if(buddyNode != NULL){ //If buddy is free (need to merge)
        TNode* otherLinkedListNode = find_node(_memlist, buddyNode->key);

        if(linkedListNode->key < buddyNode->key){
            delete_node(
                &_memlist,
                otherLinkedListNode
            ); //Handles all linking of nodes

            otherLinkedListNode = NULL; //So otherLinkedListNode is not a dangling ptr
        } else{
            delete_node(
                &_memlist,
                linkedListNode
            ); //Handles all linking of nodes

            linkedListNode = otherLinkedListNode;
        }

        linkedListNode->length <<= 1;

        delete_node(
            buddySystemArr + pow,
            buddyNode
        ); //Handles all linking of nodes

        ++pow; //Move up a size lvl

        if(linkedListNode->length < MEMSIZE){
            goto CheckBuddy;
        }

        goto NoMoreBuddy;
    } else{
NoMoreBuddy:
        node = make_node(0, NULL);

        node->key = linkedListNode->key;

        insert_node(buddySystemArr + pow, node, ASCENDING);

        linkedListNode->isTaken = '0'; //Done when linkedListNode is finalized ver
    }

    //* Depopulate heap with mem units deallocated (can use...)
    for(size_t i = node->key; i < node->key + (1 << (pow + 10)); ++i){
        _heap[i] = '0';
    }
    //*/
}

