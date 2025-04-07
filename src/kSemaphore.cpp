//
// Created by os on 6/27/22.
//

#include "../h/kSemaphore.h"

uint64 kSemaphore::idTracker = 0;
kSemaphore::Elem* kSemaphore::shead = 0;

int kSemaphore::createSem(sem_t *handle, unsigned int init) {
    kSemaphore* newSem = (kSemaphore*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kSemaphore)));
    if (newSem == 0) return -1; //Nema prostora za alokaciju novog semafora

    uint64 sid = ++kSemaphore::idTracker;
    newSem->id = sid;
    newSem->val = init;
    newSem->blocked = 0;

    kSemaphore::Elem* newElem = (kSemaphore::Elem*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kSemaphore::Elem)));
    if (newElem == 0) {
        MemoryAllocator::free(newSem);
        return -2; //Nema prostora za ulancavanje novog semafora u listu semafora
    }
    newElem->info = newSem;
    newElem->next = kSemaphore::shead;

    *handle = (_sem*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(_sem)));
    if (*handle == 0) {
        MemoryAllocator::free(newSem);
        MemoryAllocator::free(newElem);
        return -3; //Nema prostora za alokaciju handle-a
    }

    (*handle)->id = sid;
    kSemaphore::shead = newElem;

    return 0;
}

kSemaphore *kSemaphore::getSem(sem_t handle) {
    if (handle == 0) return 0;
    kSemaphore::Elem* curr = kSemaphore::shead;

    while(curr) {
        if (curr->info->id == handle->id) return curr->info;
        curr = curr->next;
    }

    return 0;
}

int kSemaphore::wait(sem_t handle) {
    kSemaphore* sem = kSemaphore::getSem(handle);
    if (sem == 0) return -1; //Nepostojeci semafor

    if (sem->val == 0) {
        kSemaphore::ThreadElem* newElem = (kSemaphore::ThreadElem*)MemoryAllocator::allocate(MemoryAllocator::sizeInBlocks(sizeof(kSemaphore::ThreadElem)));
        if (newElem == 0) {
            return -2; //Nema prostora za ulancavanje niti u listu blokiranih niti
        }

        newElem->info = kThread::running;
        newElem->next = 0;

        kSemaphore::ThreadElem* tail = sem->blocked;
        if (tail == 0) sem->blocked = newElem;
        else {
            while(tail->next != 0) tail = tail->next;
            tail->next = newElem;
        }
        __asm__ volatile ("addi sp, sp, -16");
        __asm__ volatile ("sd %0, 8(sp)" : : "r" (handle->id));

        kThread::yield(kThread::running, Scheduler::get());

        uint64 semId;
        __asm__ volatile ("ld %0, 8(sp)" : "=r" (semId));
        __asm__ volatile ("addi sp, sp, 16");

        if (kSemaphore::exists(semId)) return 0;
        return -3; //Semafor zatvoren dok je nit bila blokirana
    }
    else sem->val--;

    return 0;
}

int kSemaphore::signal(sem_t handle) {
    kSemaphore* sem = kSemaphore::getSem(handle);
    if (sem == 0) return -1; //Nepostojeci semafor

    if (sem->blocked == 0) {
        sem->val++;
        return 0;
    }

    kThread* blockedThread = sem->blocked->info;
    kSemaphore::ThreadElem* nextElem = sem->blocked->next;

    MemoryAllocator::free(sem->blocked);

    sem->blocked = nextElem;
    Scheduler::put(blockedThread);

    return 0;
}

int kSemaphore::close(sem_t handle) {
    kSemaphore* sem = kSemaphore::getSem(handle);
    if (sem == 0) return -1; //Nepostojeci semafor

    while (sem->blocked != 0) {
        kThread* blockedThread = sem->blocked->info;
        kSemaphore::ThreadElem* nextElem = sem->blocked->next;

        MemoryAllocator::free(sem->blocked);

        sem->blocked = nextElem;
        Scheduler::put(blockedThread);
    }

    kSemaphore::Elem* curr = kSemaphore::shead;
    kSemaphore::Elem* prev = 0;

    while (curr && curr->info != sem) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == 0) return -1; // Nepostojeci semafor???
    if (prev == 0) kSemaphore::shead = curr->next;
    else prev->next = curr->next;

    MemoryAllocator::free(curr);
    MemoryAllocator::free(sem);

    return 0;
}

bool kSemaphore::exists(uint64 id) {
    kSemaphore::Elem* curr = kSemaphore::shead;
    while (curr) {
        if (curr->info->id == id) return true;
        curr = curr->next;
    }
    return false;
}

