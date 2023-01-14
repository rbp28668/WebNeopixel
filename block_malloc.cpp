#include "block_malloc.hpp"

#include "pico/stdlib.h"
#include "pico/printf.h"

Block::Block()
: allocated(false)
, used_count(0)
, free_count(BLOCK_SIZE)
, activeBlock(this)
, nextBlock(0)
, guard(0xAAAAAAAA)
{
    for(int i=0; i< BLOCK_SIZE; ++i) block[i] = 0;
}

/// @brief Helper function to allocate from a single block.
/// @param bytes is the number of bytes wanted.
/// @return pointer to allocated memory or 0 if this block cannot provide.
void* Block::localAlloc(size_t bytes) {
    void* mem = 0;
    if(free_count >= bytes){
        mem = block + used_count;
        used_count += bytes;
        free_count -= bytes;
    }
    return mem;
}

/// @brief Allocate a number of bytes from the chain of blocks.
/// Note that when a block is allocated, it will pull further blocks
/// from the pool to satisfy requests.  This is transparent - the 
/// blocks are linked but allocation requests continue to go to the 
/// initial head block.
/// @param bytes is the number of bytes to allocate.
/// @return pointer to allocated memory or 0 if cannot allocate.
void* Block::allocate(size_t bytes){
    if(bytes > BLOCK_SIZE) return 0; // can never satisfy this

    void* mem = activeBlock->localAlloc(bytes);
    if(mem == 0){
        Block* next = pool->allocate();
        if(next) {
            activeBlock->nextBlock = next;  // add new block to tail.
            activeBlock = next;             // it's the new block to allocate from.
            mem = activeBlock->localAlloc(bytes);
        }
    }    
    return mem;
 }

/// @brief  Called when a block is allocated from the pool to reinitialise the block.
/// @param pool is the source pool for allocation - kept for chaining blocks.
void Block::allocateBlock(BlockPool* pool){
    this->pool = pool;
    this->used_count = 0;
    this->free_count = BLOCK_SIZE;
    this->nextBlock = 0;
    this->activeBlock = this; // start of chain
    this->allocated = true;
}

 /// @brief Frees the block and any chained blocks.
 void Block::free(){
    this->allocated = false;
    for(int i=0; i< BLOCK_SIZE; ++i) block[i] = 0;
    this->used_count = 0;
    this->free_count = BLOCK_SIZE;
    this->activeBlock = this;   
    if(this->nextBlock) this->nextBlock->free();
    this->nextBlock = 0;
 }

BlockPool::BlockPool()
:position(0)
{
}

/// @brief Looks for a block marked as unallocated.
/// @return newly allocated block or 0 if all in use.
Block* BlockPool::allocate(){
    printf("Allocating from block pool at %08x\n", this);
    Block* found = 0;
    for(int i=0; i<BLOCK_COUNT; ++i) {
        if(blocks[position].isAllocated()) {
            position = (position + 1) % BLOCK_COUNT;
        } else { // not allocated
            found = blocks + position;
            found->allocateBlock(this);
            ++position; // for next time
            break;
        }
    }
    printf("Allocated block at %08x\n", found);
    return found;
}

