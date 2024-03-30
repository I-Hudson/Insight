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
                m_lock.Lock();
            }
            ~ScopedLock()
            {
                m_lock.Unlock();
            }

        private:
            TLock& m_lock;
        };
    }
}