#pragma once

#include "pool_allocator_port_api.h"
#include "rtos_wrapper/rtos.h"
#include "singelton/singelton.h"

namespace allocator {

struct PoolAllocatorPort : public Singleton<PoolAllocatorPort>, public IPoolAllocatorPort {
friend class Singleton<PoolAllocatorPort>;
public:
    virtual void AllocatorPortEnterCriticalSection(void) override {
        wrtos::Rtos::EnterCriticalSection();
    }

    virtual void AllocatorPortExitCriticalSection(void) override {
        wrtos::Rtos::LeaveCriticalSection();
    }
};

}