#include "Memory/NewDeleteOverload.h"

#include "Core/MemoryTracker.h"

#ifdef IS_MEMORY_OVERRIDES
void* operator new(const size_t size)
{
    if (size == 0)
    {
        return nullptr;
    }

    if (void* ptr = std::malloc(size))
    {
        Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Single, Insight::Core::MemoryAllocCategory::Unknown);
        return ptr;
    }
    return nullptr;
}

void* operator new[](const size_t size)
{
    if (size == 0)
    {
        return nullptr;
    }

    if (void* ptr = std::malloc(size))
    {
        Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Array, Insight::Core::MemoryAllocCategory::Unknown);
        return ptr;
    }
    return nullptr;
}

void operator delete(void* ptr)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
    std::free(ptr);
}

void operator delete(void* ptr, u64 bytes)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
    std::free(ptr);
}

void operator delete[](void* ptr)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
    std::free(ptr);
}
void operator delete[](void* ptr, u64 bytes)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
    std::free(ptr);
}
#endif