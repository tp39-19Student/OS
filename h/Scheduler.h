//
// Created by os on 6/23/22.
//

#include "../h/kThread.h"
#ifndef PROJECT_BASE_SCHEDULER_H
#define PROJECT_BASE_SCHEDULER_H

class Scheduler {
private:
    struct Elem {
        kThread* info;
        Elem* next;
    } static *threads;

    static kThread* idleThread;
    static void idleFunction(void* arg);

    Scheduler(){}
public:
    static void put(kThread* thread);
    static kThread* get();

    static void init();
};

#endif //PROJECT_BASE_SCHEDULER_H
