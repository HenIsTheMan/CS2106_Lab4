///Whole implementation goes against conventional buddy system covered in lecture but this is what they want for the lab

#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include "llist.h"

char _heap[MEMSIZE] = {0};

///Stores node->length in KiB
TNode *_memlist = NULL; // To maintain information about length (actual length of allocated mem blks)

///Contains all buddies (taken and free, have fixed lengths of pows of 2) so "explicit"
///Stores node->length in KiB
///Will have internal mem fragmentation
TNode* explicitBuddySystemArr[MAX_ORDER] = {0}; //Init all elements to NULL

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

    TNode* node = find_node(_memlist, (unsigned int)get_index(ptr));

    if(node == NULL || node->isTaken == '0'){
        return -1;
    }

    return node->length << 10; //<< 10 for conversion from KiB to bytes
}

void print_memlist() { //Does not reflect size of each allocated blk of mem but this is what they want for the lab
    TNode* node;

    for(int i = MAX_ORDER - 1; i >= 0; --i){
        printf("Block size %d KiB:", 1 << i);

        node = explicitBuddySystemArr[i]; //node->length is in KiB for buddies

        //* Loop Unrolling applied
        if(node != NULL){
            printf(" %s, %u, %zu ->",
                node->isTaken == '1' ? "ALLOCATED" : "FREE",
                node->key,
                node->length //same as 1 << i
            );

            node = node->next;

            if(node != NULL){
                printf(" %s, %u, %zu ->",
                    node->isTaken == '1' ? "ALLOCATED" : "FREE",
                    node->key,
                    node->length //same as 1 << i
                );
            }
        }
        //*/

        puts("");
    }
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) { //size is in bytes
    if(size > MEMSIZE){ //If mem blk to alloc cannot fit entire mem
        return NULL; //No space to allocate
    }

    unsigned int startIndex = 0; //Init to 0 since explicitBuddySystemArr[i] == NULL is 1st run
    size_t buddyNodeLength;
    size_t sizeInKib = size >> 10; //>> 10 for conversion from bytes to KiB
    char isAllocated = '0';

    for(int i = MAX_ORDER - 1; i >= 0; --i){
        buddyNodeLength = 1 << i;

        if(sizeInKib > buddyNodeLength){ //If sizeInKib cannot fit into curr set of buddy partitions (existence depends on explicitBuddySystemArr[i] == NULL)
            if(isAllocated == '0'){
                return NULL; //No space to allocate
            }
            
            break;
        }

        if(explicitBuddySystemArr[i] == NULL){ //Creation of buddies, Lazy Init for 1st run (no other way in this implementation)
            //* Create left buddy 1st
            TNode* buddyNode = make_node(startIndex, NULL); //Starting address of buddyNode is 0 since explicitBuddySystemArr[i] == NULL

            buddyNode->isTaken = '1'; //Always taken since left buddy and explicitBuddySystemArr[i] == NULL
            buddyNode->length = buddyNodeLength; //In KiB

            insert_node(explicitBuddySystemArr + i, buddyNode, ASCENDING);
            //*/

            //* Create right buddy if needed
            if(i != MAX_ORDER - 1){
                buddyNode = make_node(startIndex + buddyNodeLength, NULL); //Starting address of buddyNode is buddyNodeLength since...

                buddyNode->isTaken = '0'; //Always free since right buddy and explicitBuddySystemArr[i] == NULL
                buddyNode->length = buddyNodeLength; //In KiB

                insert_node(explicitBuddySystemArr + i, buddyNode, ASCENDING);
            }
            //*/

            isAllocated = '1';
        } else{
            ///explicitBuddySystemArr[i]->next definitely exists for i != MAX_ORDER - 1 due to Creation of buddies
            if((i == MAX_ORDER - 1 && explicitBuddySystemArr[i]->isTaken == '1')
                || (i != MAX_ORDER - 1 && explicitBuddySystemArr[i]->isTaken == '1' && explicitBuddySystemArr[i]->next->isTaken == '1')
            ){ //If only buddy is taken or both buddies are taken
                continue;
            }

            //* If only buddy is not taken or both buddies are not taken
            ///2 cases for i = MAX_ORDER - 1: [if] FREE; [do nth] TAKEN;
            ///3 cases for i != MAX_ORDER - 1: [if] FREE, FREE; [if] FREE, TAKEN; [else if] TAKEN, FREE;
            if(explicitBuddySystemArr[i]->isTaken == '0'){ //Mark left partition as taken (means completely taken or 1 of its child partitions is taken) 1st if possible as 1st Fit
                //* 1st Fit so no need to explore case where right partition is taken
                explicitBuddySystemArr[i]->isTaken = '1';

                startIndex = explicitBuddySystemArr[i]->key;
                //*/
            } else if(i != MAX_ORDER - 1){ //explicitBuddySystemArr[i]->next definitely exists...
                if(explicitBuddySystemArr[i - 1] == NULL || (explicitBuddySystemArr[i - 1] != NULL
                    && (sizeInKib > explicitBuddySystemArr[i - 1]->length
                    || (explicitBuddySystemArr[i - 1]->isTaken == '1' && explicitBuddySystemArr[i - 1]->next->isTaken == '1'))
                )){
                    //* Mark right partition of TAKEN, FREE as taken (means...)
                    explicitBuddySystemArr[i]->next->isTaken = '1';

                    startIndex = explicitBuddySystemArr[i]->next->key;
                    //*/
                } else{
                    startIndex = explicitBuddySystemArr[i]->key; //Go from left partition
                }
            }

            isAllocated = '1';
            //*/
        }
    }

    //* Alloc mem blk in _memlist
    TNode* node = find_node(_memlist, startIndex);

    if(node == NULL){
        node = make_node(startIndex, NULL);

        node->isTaken = '1';
        node->length = sizeInKib;

        insert_node(&_memlist, node, ASCENDING);
    } else{
        node->isTaken = '1';
        node->length = sizeInKib;
    }
    //*/

    //* Populate heap with units allocated
    for(size_t i = startIndex; i < size; ++i){
        _heap[i] = '1';
    }
    //*/

    return _heap + startIndex; //Returns starting address of allocated mem blk
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
    if(ptr == NULL){
        return; //Fail silently
    }

    unsigned int startIndex = (unsigned int)get_index(ptr);

    TNode* node = find_node(_memlist, startIndex);

    if(node == NULL || node->isTaken == '0'){ //ptr "does not point to a memory region created by mymalloc"
        return; //Fail silently
    }

    node->isTaken = '0';

    //* Depopulate heap with actual units deallocated
    for(size_t i = startIndex; i < node->length; ++i){
        _heap[i] = '0';
    }
    //*/

    int powOfGequalNearestPowOfTwo = -1; //Init with invalid val

    while(node->length > (1 << ++powOfGequalNearestPowOfTwo)){
    }

    //int index;

    //for(index = MAX_ORDER - 1; index >= 0; --index){ //Shld have fastest avg case if start from the end
    //    node = find_node(explicitBuddySystemArr[index], startIndex);

    //    if(node != NULL){
    //        break;
    //    }
    //}

    //node->isTaken = '0';

    //if(node == explicitBuddySystemArr[index]){ //If left partition
    //    if(node->isTaken
    //} else{
    //}
}

