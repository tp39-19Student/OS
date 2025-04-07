//
// Created by os on 6/24/22.
//

#include "../h/Scheduler.h"
#include "../h/syscall_c.h"

Scheduler::Elem* Scheduler::threads = 0;
kThread* Scheduler::idleThread = 0;

void Scheduler::put(kThread *thread) {
    if (thread == 0) return;
    if (thread == idleThread) return;
    Scheduler::Elem* newElement = (Scheduler::Elem*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(Scheduler::Elem)));

    newElement->next = 0;
    newElement->info = thread;

    Scheduler::Elem* curr = Scheduler::threads;
    while (curr != 0 && curr->next != 0) curr = curr->next;

    if (curr == 0) Scheduler::threads = newElement;
    else curr->next = newElement;
}

kThread *Scheduler::get() {
    if (Scheduler::threads == 0) return idleThread;

    kThread* ret = Scheduler::threads->info;
    Scheduler::Elem* next = Scheduler::threads->next;

    MemoryAllocator::free(Scheduler::threads);
    Scheduler::threads = next;

    return ret;
}

void Scheduler::init() {
    if (idleThread != 0) return;

    thread_t handle;
    thread_create(&handle, Scheduler::idleFunction, 0);

    idleThread = kThread::getThread(handle);
}

void Scheduler::idleFunction(void* arg) {
    while(1) thread_dispatch();
}
