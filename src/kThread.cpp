//
// Created by os on 6/24/22.
//

#include "../h/kThread.h"
#include "../h/syscall_c.h"
#include "../h/Scheduler.h"
uint64 kThread::idTracker = 1;
kThread::Elem* kThread::thead = 0;
kThread* kThread::running = 0;

extern "C" void threadWrapper(void (*f)(void*), void* arg);

int kThread::createThread(thread_t *handle, void (*start_routine)(void *), void *arg, void *stack_space) {
    kThread* newThread = (kThread*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kThread)));
    if (newThread == 0) return -1; //Nema prostora za alokaciju nove niti

    uint64 tid = ++kThread::idTracker;
    newThread->id = tid;

    kThread::Elem* newElem = (kThread::Elem*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kThread::Elem)));
    if (newElem == 0) {
        MemoryAllocator::free(newThread);
        return -2; //Nema prostora za ulancavanje nove niti u listu niti
    }
    newElem->info = newThread;
    newElem->next = kThread::thead;

    newThread->sp = (uint64)stack_space;
    newThread->stackStart = (void*)((uint64)stack_space - DEFAULT_STACK_SIZE + 1);
    if (newThread->sp % 16 != 0) newThread->sp -= newThread->sp % 16;

    newThread->pc = (uint64)&threadWrapper;
    newThread->pc -= 4;

    newThread->sp -= 256;

    //sd x2, 0x10(sp)
    //sd x10, 0x50(sp)
    //sd x11, 0x58(sp)
    //sd x12, 0x60(sp)

    //ld x3, 0x18(sp)
    //ld x4, 0x20(sp)
    __asm__ volatile ("sd %[sps], 0x10(%[sp])" : : [sps] "r" (newThread->sp), [sp] "r" (newThread->sp));
    __asm__ volatile ("sd %[f], 0x58(%[sp])" : : [f] "r" (start_routine), [sp] "r" (newThread->sp));
    __asm__ volatile ("sd %[arg], 0x60(%[sp])" : : [arg] "r" (arg), [sp] "r" (newThread->sp));

    __asm__ volatile ("sd x0, 0x18(%[sp])" : : [sp] "r" (newThread->sp));
    __asm__ volatile ("sd x0, 0x20(%[sp])" : : [sp] "r" (newThread->sp));

    //handleSupervisorTrap stack
    //Return adresa za handleSupervisorTrap
    //ra na 8(sp), trenutno se nalazi na 8(s0)
    newThread->sp -= 16;
    uint64 ra;
    __asm__ volatile ("ld %0, 8(s0)" : "=r" (ra));
    __asm__ volatile ("sd %[ra], 8(%[sp])" : : [ra] "r" (ra), [sp] "r" (newThread->sp));

    //Return adresa za yield
    //ra na newThread->raYield, trenutno se nalazi na -8(s0)
    __asm__ volatile ("ld %0, -8(s0)" : "=r" (ra));
    newThread->raYield = ra;


    //yield stack
    newThread->sp -= 16;

    *handle = (_thread*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(_thread)));
    if (*handle == 0) {
        MemoryAllocator::free(newThread);
        MemoryAllocator::free(newElem);
        return -3; //Nema prostora za alokaciju handle-a
    }

    kThread::thead = newElem;

    Scheduler::put(newThread);

    (*handle)->id = tid;

    return 0;
}

kThread* kThread::getThread(thread_t handle) {
    if (handle == 0) return 0;
    kThread::Elem* curr = kThread::thead;

    while(curr) {
        if (curr->info->id == handle->id) return curr->info;
        curr = curr->next;
    }

    return 0;
}

void kThread::yield(kThread *oldThread, kThread *newThread) {
    if (oldThread == newThread) return;

    if (oldThread != 0) {
        __asm__ volatile ("csrr %0, sepc" : "=r" (oldThread->pc));
        __asm__ volatile ("mv %0, sp" : "=r" (oldThread->sp));
        __asm__ volatile ("mv %0, ra" : "=r" (oldThread->raYield));
    }
    if (newThread != 0) {
        kThread::running = newThread;

        __asm__ volatile ("csrw sepc, %0" : : "r" (newThread->pc));
        __asm__ volatile ("mv sp, %0" : : "r" (newThread->sp));
        __asm__ volatile ("mv ra, %0" : : "r" (newThread->raYield));
    }
}

void kThread::init() {
    if (kThread::thead != 0) return;
    kThread* mainThread = (kThread*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kThread)));
    mainThread->id = 1;


    kThread::Elem* mainElem = (kThread::Elem*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kThread::Elem)));
    mainElem->next = 0;
    mainElem->info = mainThread;

    kThread::thead = mainElem;
    kThread::running = mainThread;
}

void kThread::exitThread() {
    kThread* t = kThread::running;
    if (t == 0) return;

    kThread::Elem* curr = kThread::thead;
    kThread::Elem* prev = 0;

    while (curr && curr->info != t) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == 0) return;
    if (prev == 0) kThread::thead = curr->next;
    else prev->next = curr->next;

    MemoryAllocator::free(curr);
    MemoryAllocator::free(t->stackStart);
    MemoryAllocator::free(t);

    kThread::running = 0;

    kThread::yield(kThread::running, Scheduler::get());
}

void kThread::dispatch() {
    Scheduler::put(kThread::running);
    kThread::yield(kThread::running, Scheduler::get());
}
