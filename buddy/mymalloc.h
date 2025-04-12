#define MEMSIZE 1024 * 1024							// Size of memory in bytes
#define MAX_ORDER 11								// Max memory size is 1024KB, smallest block is 1KB
#define LOG_MINIMUM_BLOCK 10						// 1 KB minimum block size order (log of MINIMUM_BLOCK_SIZE to base 2)
#define MINIMUM_BLOCK_SIZE 1 << LOG_MINIMUM_BLOCK 	// Corresponds to size of 1 allocation unit in bytes (1024 in this case)

long get_index(void *ptr);

long get_size(void *ptr);

void print_memlist(); //In terms of Buddy System

void *mymalloc(size_t);

void myfree(void *);
