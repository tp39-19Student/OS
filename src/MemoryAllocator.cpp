//
// Created by os on 6/20/22.
//
#include "../h/MemoryAllocator.h"

void *MemoryAllocator::allocate(size_t size) {
    if (size == 0) return 0;
    MemoryAllocator::FreeFragment* current = MemoryAllocator::freeMemHead;
    MemoryAllocator::FreeFragment* prev = 0;

    while (current != 0 && current->sizeInBlocks < size) {
        prev = current;
        current = current->next;
    }

    if (current == 0) return 0; //Nema prostora

    if (current->sizeInBlocks > size) {
        MemoryAllocator::FreeFragment* newFragment = (MemoryAllocator::FreeFragment*)((uint64)current + size * MEM_BLOCK_SIZE);

        newFragment->next = current->next;
        newFragment->sizeInBlocks = current->sizeInBlocks - size;
        if (prev != 0) prev->next = newFragment;
        else MemoryAllocator::freeMemHead = newFragment;
    }
    else {
        if (prev != 0) prev->next = current->next;
        else MemoryAllocator::freeMemHead = current->next;
    }

    void* ret = (void*)((uint64)current + sizeof(size_t));
    *((size_t*)current) = size;

    return ret;
}

int MemoryAllocator::free(void *pointer) {
    if (pointer < HEAP_START_ADDR || pointer > HEAP_END_ADDR) return -1; //Objekat nije u dinamickoj zoni alokacije
    size_t size = *((size_t*)((uint64)pointer - sizeof(size_t)));
    if (size <= 0 || size > ((uint64)HEAP_END_ADDR - (uint64)HEAP_START_ADDR) / MEM_BLOCK_SIZE) return -2; //Objekat nije alocirao mem_alloc

    MemoryAllocator::FreeFragment* newFragment = (MemoryAllocator::FreeFragment*)((uint64)pointer - sizeof(size_t));
    newFragment->sizeInBlocks = size;

    MemoryAllocator::FreeFragment* prev = 0;
    MemoryAllocator::FreeFragment* curr = MemoryAllocator::freeMemHead;

    while(curr != 0 && curr < newFragment) {
        prev = curr;
        curr = curr->next;
    }

    if (prev != 0) {
        prev->next = newFragment;
    } else {
        MemoryAllocator::freeMemHead = newFragment;
    }
    newFragment->next = curr;

    if(tryToJoin(prev)) tryToJoin(prev);
    else tryToJoin(newFragment);

    return 0;
}

MemoryAllocator::FreeFragment* MemoryAllocator::freeMemHead = 0;

void MemoryAllocator::init() {
    /*
    void* current = (void*)HEAP_START_ADDR;
    void* next = (void*)((size_t)current + MEM_BLOCK_SIZE);
    if (next >= HEAP_END_ADDR) {
        MemoryAllocator::freeMemHead = 0;
        return;
    }

    while(next < HEAP_END_ADDR) {
        ((MemoryAllocator::FreeFragment*)current)->next = (MemoryAllocator::FreeFragment*)next;
        current = next;
        next = (void*)((size_t)next + MEM_BLOCK_SIZE);
    }

    ((MemoryAllocator::FreeFragment*)current)->next = 0;*/

    MemoryAllocator::freeMemHead = (MemoryAllocator::FreeFragment*)HEAP_START_ADDR;
    MemoryAllocator::freeMemHead->next = 0;
    MemoryAllocator::freeMemHead->sizeInBlocks = ((uint64)HEAP_END_ADDR - (uint64)HEAP_START_ADDR) / MEM_BLOCK_SIZE;
}

int MemoryAllocator::tryToJoin(MemoryAllocator::FreeFragment *prev) {
    if (prev == 0 || prev->next == 0) return 0;
    if ((MemoryAllocator::FreeFragment*)((uint64)prev + prev->sizeInBlocks * MEM_BLOCK_SIZE) == prev->next) {
        prev->sizeInBlocks += prev->next->sizeInBlocks;
        prev->next = prev->next->next;
        return 1;
    }
    return 0;
}

bool MemoryAllocator::isAllocated(void *pointer) {
    if (!(pointer != 0 && pointer > HEAP_START_ADDR && pointer < HEAP_END_ADDR)) return false;

    size_t size = *((size_t*)((uint64)pointer - sizeof(size_t)));
    if (size <= 0 || size > ((uint64)HEAP_END_ADDR - (uint64)HEAP_START_ADDR) / MEM_BLOCK_SIZE) return false;

    return true;
}

size_t MemoryAllocator::sizeInBlocks(size_t sizeInBytes) {
    size_t ret = sizeInBytes + sizeof(size_t);
    ret = ((ret - (ret % MEM_BLOCK_SIZE)) / MEM_BLOCK_SIZE) + (ret%MEM_BLOCK_SIZE > 0?1:0);

    return ret;
}