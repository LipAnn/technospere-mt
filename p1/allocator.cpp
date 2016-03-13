#include "allocator.h"
#include <algorithm>
#include <utility>
#include <iostream>
#include <cstring>

using std::swap;
using std::min;
using std::make_pair;
using std::memcpy;

Block::Block(char *b, size_t s) : begin(b), size(s) {}

inline bool Block::empty() const {
    return size == 0;
}

inline char* Block::getEnd() const {
    return begin + size;
}

Pointer::Pointer(Allocator *alc, size_t id_p) : allocator_(alc), id_(id_p) {}

void* Pointer::get() const {
    return allocator_ == nullptr ? nullptr : allocator_->findBegin(id_);
}

inline size_t Pointer::getSize() const {
    return allocator_->findSize(id_);
}

inline size_t Pointer::getId() const {
    return id_;
}


Allocator::Allocator(void *base, size_t size) {
    Block all_memory(static_cast<char*>(base), size / 2);
    free_blocks_.push_back(all_memory);
    base1_ = static_cast<char*>(base);
    base2_ = static_cast<char*>(base) + size / 2;
    memory_size1_ = size / 2;
    memory_size2_ = size - size / 2;
    pointer_id_ = 1;
}

Block Allocator::findBlock(size_t id) {
    PointerIterator it = pointers_.find(id);
    if ((it = pointers_.find(id)) == pointers_.end()) {
        return Block();
    }
    return it->second;
}

void* Allocator::findBegin(size_t id) {
    Block bl = findBlock(id);
    return static_cast<void*>(bl.begin);
}

size_t Allocator::findSize(size_t id) {
    return findBlock(id).size;
}

inline void Allocator::splitBlock(BlockIterator it, size_t N) {
    Block new_block(it->begin + N, it->size - N); 
    auto temp_it = free_blocks_.erase(it);
    if (new_block.empty()) {
        return;
    }
    free_blocks_.insert(temp_it, new_block);
}

inline void Allocator::extendBlock(BlockIterator it, size_t N) {
    it->size += N;
}

inline Allocator::BlockIterator Allocator::findNearestFreeBlock(Pointer &p) {
    if (free_blocks_.empty()) {
        return free_blocks_.end();
    }
    Block pointer_info = findBlock(p.getId());
    for (auto it = free_blocks_.begin(); it != free_blocks_.end(); ++it) {
        if (pointer_info.getEnd() <= it->begin) {
            return it;
        }
    }
    return free_blocks_.end();
}

inline Allocator::BlockIterator Allocator::addFreeBlock(Pointer &p) {
    size_t cur_size = p.getSize();
    if (cur_size == 0) {
        return free_blocks_.begin();
    }
    auto it = findNearestFreeBlock(p);
    Block new_free_block = findBlock(p.getId());
    return free_blocks_.insert(it, new_free_block);
}

inline Allocator::BlockIterator Allocator::merge2Blocks(BlockIterator it1, BlockIterator it2) {
    size_t add_size = it2->size;
    splitBlock(it2, add_size);
    extendBlock(it1, add_size);
    return it1;
}

inline void Allocator::mergeBlocks(BlockIterator it) {
    auto prev_it = it, next_it = it;
    prev_it--;
    next_it++;
    if (free_blocks_.size() <= 1) {
        return;
    }
    if (it == free_blocks_.begin()) {
        if (it->getEnd() == next_it->begin) {
            merge2Blocks(it, next_it);
        }
    } else if (it == free_blocks_.end()) {
        if (prev_it->getEnd() == it->begin) {
            merge2Blocks(prev_it, it);
        }
    } else {
        if (it->getEnd() == next_it->begin) {
            it = merge2Blocks(it, next_it);
        }
        if (prev_it->getEnd() == it->begin) {
            merge2Blocks(prev_it, it);
        }
    }
}

Pointer Allocator::alloc(size_t N) {
    if (N == 0) {
        return Pointer();
    }

    for (auto it = free_blocks_.begin(); it != free_blocks_.end(); ++it) {
        if (it->size >= N) {
            Pointer pointer(this, pointer_id_);
            pointers_.insert(make_pair(pointer_id_++, Block(it->begin, N)));
            splitBlock(it, N);
            return pointer;
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "No free blocks");
}

inline void Allocator::erasePointer(Pointer &p) {
    if (pointers_.erase(p.getId())) {
        return;
    }

    throw AllocError(AllocErrorType::InvalidFree, "Invalid memory free");
}


void Allocator::free(Pointer &p) {
    if (p.get() == nullptr) {
        return;
    }
    auto it = addFreeBlock(p);
    mergeBlocks(it);
    erasePointer(p);
    p = Pointer();  
}

void Allocator::realloc(Pointer &p, size_t N) {
    if (p.get() == nullptr) {
        p = alloc(N);
        return;
    }
    size_t prev_size = p.getSize();
    memcpy(base2_, p.get(), prev_size);
    free(p);
    p = alloc(N);
    if (p.get()) { 
        memcpy(p.get(), base2_, min(prev_size, N));
    }
}



void Allocator::defrag() {
    char *cur_begin = base2_;
    size_t all_size = 0;
    map<size_t, Block> temp_pointers;
    for (auto it = pointers_.begin(); it != pointers_.end(); ++it) {
        all_size += it->second.size;
        memcpy(cur_begin, it->second.begin, it->second.size);
        temp_pointers.insert(make_pair(it->first, Block(cur_begin, it->second.size)));
        cur_begin = base2_ + all_size;
    }
    free_blocks_.clear();
    free_blocks_.push_back(Block(cur_begin, memory_size2_ - all_size));
    swap(base1_, base2_);
    swap(memory_size1_, memory_size2_);
    pointers_ = temp_pointers;
}




