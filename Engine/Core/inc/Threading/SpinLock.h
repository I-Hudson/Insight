#pragma once

#include "Core/Defines.h"

#include <atomic>

namespace Insight
{
    namespace Threading
    {
        class IS_CORE SpinLock
        {
        public:
            FORCE_INLINE void lock()
            {
                while (m_state.test_and_set(std::memory_order_acquire))
                {
                    PROCESSER_PAUSE;
                }
            }

            FORCE_INLINE void unlock()
            {
                m_state.clear(std::memory_order_release);
            }

        private:
            std::atomic_flag m_state = ATOMIC_FLAG_INIT;
        };
    }
}