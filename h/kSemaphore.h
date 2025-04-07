//
// Created by os on 6/27/22.
//
#include "../lib/hw.h"
#include "../h/MemoryAllocator.h"
#include "../h/kThread.h"
#include "../h/Scheduler.h"
#ifndef PROJECT_BASE_KSEMAPHORE_H
#define PROJECT_BASE_KSEMAPHORE_H

class _sem {
public:
    uint64 id;
};
typedef _sem* sem_t;

class kSemaphore {
private:
    kSemaphore(){}

    struct Elem {
        kSemaphore* info;
        Elem* next;

        Elem(kSemaphore* i) {this->info = i;}
    } static *shead;

    struct ThreadElem {
        kThread* info;
        ThreadElem* next;
    } *blocked;

    static uint64 idTracker;
    uint64 id;

    unsigned val;

public:
    static int createSem(sem_t* handle, unsigned  init);

    static kSemaphore* getSem(sem_t handle);
    static int wait(sem_t handle);
    static int signal(sem_t handle);
    static int close(sem_t handle);

    static bool exists(uint64 id);
};

#endif //PROJECT_BASE_KSEMAPHORE_H
