#pragma once

namespace Insight
{
    namespace Threading
    {
        template<typename TLock>
        class ScopedLock
        {
        public:
            ScopedLock(TLock& lock)
                : m_lock(lock)
            {
                m_lock.lock();
            }
            ~ScopedLock()
            {
                m_lock.unlock();
            }

        private:
            TLock& m_lock;
        };
    }
}