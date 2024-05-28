#include "Threading/ThreadScopeLock.h"

namespace Insight
{
    namespace Threading
    {
        ThreadScopeLock::ThreadScopeLock()
        {
        }

        ThreadScopeLock::ThreadScopeLock(ThreadScopeLock&& other)
        {
        }

        ThreadScopeLock::~ThreadScopeLock()
        {
        }

        bool ThreadScopeLock::TryLock()
        {
            return false;
        }

        bool ThreadScopeLock::Lock()
        {
            return false;
        }

        bool ThreadScopeLock::Unlock()
        {
            return false;
        }

        bool ThreadScopeLock::IsLocked() const
        {
            return false;
        }

        bool ThreadScopeLock::IsLockedThisThread() const
        {
            return false;
        }

        bool ThreadScopeLock::IsLockedByThread(const std::thread::id& threadId) const
        {
            return false;
        }
    }
}