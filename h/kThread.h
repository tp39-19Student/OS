//
// Created by os on 6/23/22.
//
#include "../lib/hw.h"
#include "../h/MemoryAllocator.h"
#ifndef PROJECT_BASE_KTHREAD_H
#define PROJECT_BASE_KTHREAD_H

class _thread {
public:
    uint64 id;
};

typedef _thread* thread_t;

class kThread {
private:
    static uint64 idTracker;
    uint64 id;

    uint64 sp;
    uint64 pc;
    uint64 raYield;

    void* stackStart;

    struct Elem {
        kThread* info;
        Elem* next;

        Elem(kThread* i) {this->info = i;}
    } static *thead;
public:
    static kThread* running;

    static int createThread(thread_t* handle, void(*start_routine)(void*), void* arg, void* stack_space);
    static void exitThread();
    static void dispatch();
    static void yield(kThread* oldThread, kThread* newThread);
    static kThread* getThread(thread_t handle);

    static void init();
};




#endif //PROJECT_BASE_KTHREAD_H
