#ifndef _BLOCK_LIST_BASE_H
#define _BLOCK_LIST_BASE_H

// Simple collection classes for storing dynamic lists in blocks. Due to
// nature of blocks no reallocation can take place but more chunks can
// be added from the block.

#include "block_malloc.hpp"

#define BLOCK_LIST_CHUNK 8

class BlockListBaseIter;

class BlockListBase {
    friend class BlockListBaseIter;

    void* data[BLOCK_LIST_CHUNK];
    unsigned int used;
    BlockListBase* next;
    BlockListBase* tail;

    void addToChunk(void* item) {
        data[used++] = item;
    }

    public:
    BlockListBase();
    void* operator new(size_t size, Block* block);

    void add(Block* block, void* item);
    BlockListBaseIter iter();
};

class BlockListBaseIter {
    friend class BlockListBase;

    BlockListBase* current;
    unsigned int position;

    public:
    BlockListBaseIter(BlockListBase* start);
    void* next();
};

template <typename T> class BlockListIter: public BlockListBaseIter {
    public:
    BlockListIter(const BlockListBaseIter& base) : BlockListBaseIter(base){}
    T* next() { return static_cast<T*>(BlockListBaseIter::next());}
};

template <typename T>  class BlockList: public BlockListBase {
    public:
    void add(Block* block, T* item) {BlockListBase::add(block, item);}
    BlockListIter<T> iter() { return static_cast<BlockListIter<T>> (BlockListBase::iter());}
};



#endif