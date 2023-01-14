// Block allocator to use for deconstructing "things" (mainly HTTP requests)
// where there tend to be lots of little "new"s all of which can be deleted
// at the same time.  
// sizes are fixed could pass in an external array if need be.

#ifndef BLOCK_MALLOC_H
#define BLOCK_MALLOC_H

#include "pico/stdlib.h"

#define BLOCK_SIZE 4096
#define BLOCK_COUNT 16

class BlockPool;

class Block {
    uint8_t block[BLOCK_SIZE];
    int guard;
    uint used_count;
    uint free_count;
    bool allocated;     // whether this block is allocated or free.
    Block* nextBlock;   // chain if more than BLOCK_SIZE needed
    Block* activeBlock; // for chained, points to last block in list where mem allocated from.
    BlockPool* pool;    // Source pool to get new blocks from.

    void* localAlloc(size_t bytes);

    public:
    Block();

    void* allocate(size_t bytes);
    void allocateBlock(BlockPool* pool);
    bool isAllocated() const {return allocated;}
    void free();
};

class BlockPool {
    Block blocks[BLOCK_COUNT];  
    int position; // for round-robin allocation.
    public:
    BlockPool();
    Block* allocate();
};

#endif
