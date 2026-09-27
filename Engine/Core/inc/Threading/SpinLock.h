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
                // Try and aquire the lock.
                while (m_state.exchange(true, std::memory_order_acquire))
                {
                    // Failed acquiring the lock, load the value to see if we can acquire the lock.
                    // This is a read only and keeps the cache line shared across cores without invalidations.
                    while (m_state.load(std::memory_order_relaxed))
                    {
                        PROCESSER_PAUSE;
                    }
                }
            }

            FORCE_INLINE void unlock()
            {
                m_state.store(false, std::memory_order_release);
            }

        private:
            std::atomic_bool m_state;
        };
    }
}