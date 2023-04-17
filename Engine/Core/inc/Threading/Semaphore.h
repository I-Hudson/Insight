#pragma once

#include "Core/Defines.h"

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
        void Signal();

    private:
        unsigned short currVal, maxVal; // 65,535 max threads assumed.
        std::mutex mtx;
        std::condition_variable cv;
    };
}