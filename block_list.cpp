#include "block_list.hpp"

BlockListBase::BlockListBase()
: used(0)
, next(0)
, tail(this)
{
    for(int i=0; i<BLOCK_LIST_CHUNK; ++i) data[i] = 0;
}

void* BlockListBase::operator new(size_t size, Block* block){
    return block->allocate(size);
}

void BlockListBase::add(Block* block, void* item){
    if(tail->used >= BLOCK_LIST_CHUNK) {
        BlockListBase* n = new(block) BlockListBase();
        tail->next = n;
        tail = n;
    }
    tail->addToChunk(item);
}

BlockListBaseIter BlockListBase::iter(){
    return BlockListBaseIter(this);
}

BlockListBaseIter::BlockListBaseIter(BlockListBase* start)
: current(start)
, position(0)
{ }

void* BlockListBaseIter::next() {
    
    if(position == BLOCK_LIST_CHUNK){
        // try next chunk
        current = current->next;
        position = 0;
        if(current == 0) return 0;
    }
    
    if(position < current->used){
        void* item = current->data[position];
        ++position;
        return item;
    } else {
        return 0;
    }
}