#include <stdio.h>
#include "bitmap.h"

// IMPLEMENTED FOR YOU
// Utility function to print out an array of char as bits
// Print the entire bitmap. Arguments: The bitmap itself, and the length of the bitmap 
// Each bit of the bitmap represents 1 byte of memory. 0 = free, 1 = allocated.
// map: The bitmap itself, an array of unsigned char.
//      Each bit of the bitmap represents one byte of memory
// len: The length of the bitmap array in characters
//
// Returns: Nothing
void print_map(unsigned char *map, int len) {
    int i, j;

    for(i=0; i<len; i++) {

        unsigned char mask = 0b10000000;
        for(j=0; j<8; j++) {
            if(map[i] & mask)
                printf("1");
            else
                printf("0");
            mask = mask >> 1;
        }
        printf(" ");
    }
    printf("\n");
}

// Search the bitmap for the required number of zeroes (representing
// free bytes of memory). Returns index of first stretch of 0s
// that meet the criteria. You can use this as an index into
// an array of char that represents the process heap
// bitmap = Bitmap declared as an array of unsigned char (shld be named "map" for consistency but we just follow)
// len = Length of bitmap in characters
// num_zeroes = Length of string of 0's required
// Returns: Index to stretch of 0's of required length, -1 if no such stretch can be found

long search_map(unsigned char *bitmap, int len, long num_zeroes) {
    int cumulativeZeroCount = 0;
    int bitmask;
    int savedBitIndex = 0; //Saved index of bit in bitmap that leads a stretch of 0's (valid val starts from 0)

    for(int i = 0; i < len; ++i){
        bitmask = 0b10000000;

        for(int j = 0; j < 8; ++j){ //8 to go through each bit in bitmap[i] since size of unsigned char is 1 byte (8 bits)
            if((bitmap[i] & bitmask) == 0){
                if(++cumulativeZeroCount == num_zeroes){
                    return savedBitIndex;
                }

                if(cumulativeZeroCount == 1){
                    savedBitIndex = i * 8 + j;
                }
            } else{ //No need to reset savedBitIndex here since its old val is not used when it is updated
                cumulativeZeroCount = 0;
            }

            bitmask >>= 1; //Apply bit shift right to bitmask to check the next bit in bitmap[i] in the next iteration 
        }
    }

    return -1;
} //main

// Set map bits to 0 or 1 depending on whether value is non-zero
// map = Bitmap, declared as an array of unsigned char
// start = Starting index to mark as 1 or 0
// length = Number of bits to mark
// value = Value to mark the bits as. value = 0 marks the bits
//          as 0, non-zero marks the bits as 1
// Returns: Nothing

void set_map(unsigned char* map, long start, long length, int value) {
    int bitmask;

    int startBitmapIndex = start / 8;
    int startWithinCharIndex = start % 8;
    int endBitmapIndex = (start + length - 1) / 8;
    int endWithinCharIndex = (start + length - 1) % 8;

    for(int i = 0; i < 8; ++i){
        if(i >= startBitmapIndex && i <= endBitmapIndex){
            bitmask = 0b11111111; //Initial val

            if(i == startBitmapIndex){ //Truncation from the left as all bits in map[startBitmapIndex] might not be included in range to set
                bitmask &= 0b11111111 >> startWithinCharIndex;
            }

            if(i == endBitmapIndex){ //Truncation from the right as..., not else if as it is possible that i == startBitmapIndex == endBitmapIndex
                bitmask &= 0b11111111 << (8 - (endWithinCharIndex + 1));
            }

            if(value == 0){
                bitmask = ~bitmask; //Flip bits in bitmask

                map[i] &= bitmask; //Set relevant bits in map[i] to 0
            } else{
                map[i] |= bitmask; //Set relevant bits in map[i] to 1
            }
        }
    }
}

// IMPLEMENTED FOR YOU
// Marks a stretch of bits as "1", representing allocated memory
// map = Bitmap declared as array of unsigned char
// start = Starting index to mark
// length = Number of bits to mark as "1"
void allocate_map(unsigned char *map, long start, long length) {

    set_map(map, start, length, 1);

}

// IMPLEMENTED FOR YOU
// Marks a stretch of bits as "0", representing allocated memory
// map = Bitmap declared as array of unsigned char
// start = Starting index to mark
// length = Number of bits to mark as "0"
void free_map(unsigned char *map, long start, long length) {
    set_map(map, start, length, 0);
}

