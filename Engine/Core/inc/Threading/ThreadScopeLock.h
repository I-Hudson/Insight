#pragma once

#include <mutex>

namespace Insight
{
    namespace Threading
    {
        /// @brief RTTI lock used for scopes allowing access if the current thread has it locked.
        class ThreadScopeLock
        {
        public:
            ThreadScopeLock();
            ThreadScopeLock(const ThreadScopeLock& other) = delete;
            ThreadScopeLock(ThreadScopeLock&& other);
            ~ThreadScopeLock();

            ThreadScopeLock& operator=(const ThreadScopeLock& other) = delete;
            ThreadScopeLock& operator=(ThreadScopeLock&& other) = delete;

            bool TryLock();
            bool Lock();
            bool Unlock();

            bool IsLocked() const;
            bool IsLockedThisThread() const;
            bool IsLockedByThread(const std::thread::id& threadId) const;

        private:
            std::mutex m_mutex;
            std::thread::id m_threadId;
        };
    }
}