#include "../h/syscall_c.h"
#include "../lib/console.h"

#include "../h/MemoryAllocator.h"
#include "../h/kThread.h"

#include "../h/Scheduler.h"
#include "../h/syscall_cpp.hpp"

uint64 timerCount = 0;
extern "C" void handleSupervisorTrap(uint64 code, void* par1, void* par2, void* par3, void* par4) {
    uint64 scause;
    __asm__ volatile ("csrr %0, scause" : "=r" (scause));

    if (scause == (0x01UL << 63 | 0x01)) {
        timerCount++;
        if (timerCount >= 50) {
            timerCount = 0;
        }
    } else if (scause == 0x08 || scause == 0x09) {
        switch(code) {
            case 0x01: MemoryAllocator::allocate(*(size_t*)par1);
                break;
            case 0x02: MemoryAllocator::free(par1);
                break;
            case 0x11: kThread::createThread((thread_t*)(par1), (void (*)(void *))(par2), par3, par4);
                break;
            case 0x12: kThread::exitThread();
                break;
            case 0x13: kThread::dispatch();
                break;
            case 0x21: kSemaphore::createSem((sem_t*)par1, *(unsigned*)par2);
                break;
            case 0x22: kSemaphore::close((sem_t)par1);
                break;
            case 0x23: kSemaphore::wait((sem_t)par1);
                break;
            case 0x24: kSemaphore::signal((sem_t)par1);
                break;
        }
        uint64 t = 0;
        __asm__ volatile("csrr %0, sepc" : "=r" (t));
        t += 4;
        __asm__ volatile("csrw sepc, %0" : : "r" (t));
    }

    console_handler();
}

extern "C" void supervisorTrap();

extern "C" void threadWrapper(uint64 pl, void (*f)(void*), void* arg) {
    f(arg);
    thread_exit();
}



void main() {
    MemoryAllocator::init();
    __asm__ volatile ("csrw stvec, %0" : : "r" (&supervisorTrap));
    kThread::init();
    Scheduler::init();

    //userMain();
}