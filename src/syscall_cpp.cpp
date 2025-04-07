//
// Created by os on 6/26/22.
//
#include "../h/syscall_cpp.hpp"
#include "../h/Scheduler.h"



void* operator new (size_t size) {
    return mem_alloc(size);
}

void operator delete (void* ptr) {
    mem_free(ptr);
}

Thread::Thread(void (*body)(void *), void *arg) {
    thread_create(&myHandle, body, arg);
}

Thread::~Thread() {
    if (myHandle != 0) delete myHandle;
}

int Thread::start() {
    if (myHandle == 0) {
        thread_create(&myHandle, runWrapper, this);
        return 0;
    }
    return -1; //Nit vec pokrenuta
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t) {
    return 0;
}

Thread::Thread() { myHandle = 0; }

void Thread::runWrapper(void *arg) {
    ((Thread*)arg)->run();
}

Semaphore::Semaphore(unsigned int init) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
    delete myHandle;
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}

PeriodicThread::PeriodicThread(time_t period) {}
