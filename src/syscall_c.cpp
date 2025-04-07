//
// Created by os on 6/22/22.
//

#include "../h/syscall_c.h"

void* mem_alloc(size_t size) {
    size_t blocks = MemoryAllocator::sizeInBlocks(size);

    return syscall(0x01, &blocks, 0, 0, 0);
}

int mem_free(void* ptr) {
    return (int)(uint64)syscall(0x02, ptr, 0, 0, 0);
}


int thread_create (
        thread_t* handle,
        void(*start_routine)(void*),
        void* arg
) {
    void* stack_space = (void*)((uint64)mem_alloc(DEFAULT_STACK_SIZE) + DEFAULT_STACK_SIZE - 1);
    return (int)(uint64)syscall(0x11, handle, (void*)(start_routine), arg, stack_space);
}

int thread_exit() {
    return (int)(uint64) syscall(0x12, 0, 0, 0, 0);
}

void thread_dispatch() {
    syscall(0x13, 0, 0, 0, 0);
}

void* syscall(uint64 code, void* par1, void* par2, void* par3, void* par4) {
    __asm__ volatile("ecall");
    uint64 ret;

    __asm__ volatile ("mv %0, a0" : "=r" (ret));
    return (void*)ret;
}

#include "../lib/console.h"

char getc() {
    return __getc();
}

void putc(char c){
    return __putc(c);
}

int sem_open(sem_t *handle, unsigned int init) {
    return (int)(uint64) syscall(0x21, handle, &init, 0, 0);
}

int sem_close(sem_t handle) {
    return (int)(uint64) syscall(0x22, handle, 0, 0, 0);
}

int sem_wait(sem_t id) {
    return (int)(uint64) syscall(0x23, id, 0, 0, 0);
}

int sem_signal(sem_t id) {
    return (int)(uint64) syscall(0x24, id, 0, 0, 0);
}
