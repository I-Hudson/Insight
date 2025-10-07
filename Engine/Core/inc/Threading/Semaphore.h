#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <mutex>

namespace Insight
{
    class IS_CORE Semaphore
    {
    public:
        Semaphore(int val);
        Semaphore(int val, int max) : maxVal(max), currVal(val) {}

        inline void P() { Wait(); }
        inline void V() { Signal(); }
        void Wait();
        void Signal(const u32 count = 0);

    private:
        unsigned short currVal, maxVal; // 65,535 max threads assumed.
        std::mutex mtx;
        std::condition_variable cv;
    };
}