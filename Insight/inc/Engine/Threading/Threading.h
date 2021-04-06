#pragma once

#include "Engine/Platform/CriticalSection.h"
#include "Engine/Platform/Platform.h"

/// <summary>
/// Scope locker for critical section.
/// </summary>
class ScopeLock
{
private:

    const CriticalSection* _section;

    ScopeLock() = default;
    ScopeLock(const ScopeLock&) = delete;
    ScopeLock& operator=(const ScopeLock&) = delete;

public:

    /// <summary>
    /// Init, enters critical section
    /// </summary>
    /// <param name="section">The synchronization object to manage</param>
    ScopeLock(const CriticalSection* section)
        : _section(section)
    {
        ASSERT_LOW_LAYER(_section);
        _section->Lock();
    }

    /// <summary>
    /// Init, enters critical section
    /// </summary>
    /// <param name="section">The synchronization object to manage</param>
    ScopeLock(const CriticalSection& section)
        : _section(&section)
    {
        _section->Lock();
    }

    /// <summary>
    /// Destructor, releases critical section
    /// </summary>
    ~ScopeLock()
    {
        _section->Unlock();
    }
};