//
// Created by os on 6/20/22.
//
#include "../lib/hw.h"
#ifndef PROJECT_BASE_MEMORYALLOCATOR_H
#define PROJECT_BASE_MEMORYALLOCATOR_H

class MemoryAllocator {
private:
    MemoryAllocator(){}

    struct FreeFragment {
        FreeFragment* next;
        size_t sizeInBlocks;
    };

    static FreeFragment* freeMemHead;

    static int tryToJoin(FreeFragment* prev);

public:
    static void init();

    static void* allocate(size_t size);
    static int free(void* pointer);

    static bool isAllocated(void* pointer);

    static size_t sizeInBlocks(size_t sizeInBytes);

};

#endif //PROJECT_BASE_MEMORYALLOCATOR_H
