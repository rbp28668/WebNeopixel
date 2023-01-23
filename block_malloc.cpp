#include "block_malloc.hpp"
#ifdef BLOCK_USE_CALLOC
#include <stdlib.h>
#endif
 
#include "pico/stdlib.h"
#include "pico/printf.h"

Block::Block()
: allocated(false)
#ifdef BLOCK_USE_CALLOC
, block(0)
#endif 
, used_count(0)
, free_count(BLOCK_SIZE)
, activeBlock(this)
, nextBlock(0)
, guard0(_BLOCK_POOL_GUARD)
, guard1(_BLOCK_POOL_GUARD)
{
    #ifdef BLOCK_USE_CALLOC
    block = (uint8_t*)calloc(BLOCK_SIZE,1);
    #else
    for(int i=0; i< BLOCK_SIZE; ++i) block[i] = 0;
    #endif
 }

 Block::~Block(){
    #ifdef BLOCK_USE_CALLOC
    ::free((void*)block);
    #endif
 }

/// @brief Helper function to allocate from a single block.
/// @param bytes is the number of bytes wanted.
/// @return pointer to allocated memory or 0 if this block cannot provide.
void* Block::localAlloc(size_t bytes) {
    assert(this);
    assert(guard0 == _BLOCK_POOL_GUARD);
    assert(guard1 == _BLOCK_POOL_GUARD);
    
    // Round up to nearest 4 byte aligned value. Otherwise weird things happen.
    bytes = (bytes + 3) & ~3;
    void* mem = 0;
    if(free_count >= bytes){
        mem = block + used_count;
        used_count += bytes;
        free_count -= bytes;
    }
    assert((( (uint)mem & ~3) == (uint)mem));
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
    assert(this);
    assert(used_count <= BLOCK_SIZE);
    assert((used_count + free_count) == BLOCK_SIZE);

    assert(guard0 == _BLOCK_POOL_GUARD);
    assert(guard1 == _BLOCK_POOL_GUARD);
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
    assert(activeBlock->nextBlock == 0);    
    assert(used_count <= BLOCK_SIZE);
    assert((used_count + free_count) == BLOCK_SIZE);
    return mem;
 }

/// @brief  Called when a block is allocated from the pool to reinitialise the block.
/// @param pool is the source pool for allocation - kept for chaining blocks.
void Block::allocateBlock(BlockPool* pool){
    assert(this);
    assert(!allocated);
    assert(guard0 == _BLOCK_POOL_GUARD);
    assert(guard1 == _BLOCK_POOL_GUARD);
 
    this->pool = pool;
    this->used_count = 0;
    this->free_count = BLOCK_SIZE;
    this->nextBlock = 0;
    this->activeBlock = this; // start of chain
    this->allocated = true;
}

 /// @brief Frees the block and any chained blocks.
 void Block::free(){
    assert(this);
 
    Block* pb = this;
    while(pb){
        assert(pb->allocated);
        assert(pb->used_count <= BLOCK_SIZE);
        assert((pb->used_count + pb->free_count) == BLOCK_SIZE);
        assert(pb->guard0 == _BLOCK_POOL_GUARD);
        assert(pb->guard1 == _BLOCK_POOL_GUARD);

        pb->allocated = false;
        for(int i=0; i< BLOCK_SIZE; ++i) pb->block[i] = 0;
        pb->used_count = 0;
        pb->free_count = BLOCK_SIZE;
        pb->activeBlock = this;   
        Block* next = pb->nextBlock;
        pb->nextBlock = 0;
        pb = next;
    }
 }

BlockPool::BlockPool()
:position(0)
{
}

/// @brief Looks for a block marked as unallocated.
/// @return newly allocated block or 0 if all in use.
Block* BlockPool::allocate(){
    Block* found = 0;
    for(int i=0; i<BLOCK_COUNT; ++i) {
        if(blocks[position].isAllocated()) {
            position = (position + 1) % BLOCK_COUNT;
        } else { // not allocated
            found = blocks + position;
            found->allocateBlock(this);
            position = (position + 1) % BLOCK_COUNT; // for next time
            break;
        }
    }
    return found;
}

