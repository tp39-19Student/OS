//
// Created by os on 6/20/22.
//
#include "../lib/hw.h"
#include "../h/kThread.h"
#include "../h/kSemaphore.h"
#include "../h/MemoryAllocator.h"
#ifndef PROJECT_BASE_SYSCALL_C_H
#define PROJECT_BASE_SYSCALL_C_H

void* mem_alloc(size_t size);
int mem_free(void*);

int thread_create (
        thread_t* handle,
        void(*start_routine)(void*),
        void* arg
);
int thread_exit();
void thread_dispatch();

int sem_open (
        sem_t* handle,
        unsigned init
);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);


void* syscall(uint64 code, void* par1, void* par2, void* par3, void* par4);

const int EOF = -1;
char getc();
void putc(char);

#endif //PROJECT_BASE_SYSCALL_C_H
