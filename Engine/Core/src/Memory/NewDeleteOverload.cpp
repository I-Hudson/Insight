#include "Memory/NewDeleteOverload.h"

#include "Core/MemoryTracker.h"

#include <cstdlib>

thread_local bool gReentryGuard = false;

#ifdef IS_MEMORY_OVERRIDES
void* operator new(const size_t size)
{
    if (size == 0)
    {
        return nullptr;
    }

#if _MSC_VER
    if (void* ptr = malloc(size))
#else
    if (void* ptr = std::malloc(size))
#endif // _MSC_VER
    {
        if (gReentryGuard)
        {
            gReentryGuard = true;
            Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Single, Insight::Core::MemoryAllocCategory::Unknown);
            gReentryGuard = false;
        }
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

#if _MSC_VER
    if (void* ptr = malloc(size))
#else
    if (void* ptr = std::malloc(size))
#endif // _MSC_VER
    {
        Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Array, Insight::Core::MemoryAllocCategory::Unknown);
        return ptr;
    }
    return nullptr;
}

void operator delete(void* ptr)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

void operator delete(void* ptr, u64 bytes)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

void operator delete[](void* ptr)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

void operator delete[](void* ptr, u64 bytes)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

#if C_PLUS_PLUS_STANDARD >= 201703L
void* operator new(size_t size, std::align_val_t al)
{
    if (size == 0)
    {
        return nullptr;
    }

#if _MSC_VER
    if (void* ptr = _aligned_malloc(size, static_cast<size_t>(al)))
#else
    assert((size % al) == 0);
    if (void* ptr = std::aligned_alloc(size))
#endif // _MSC_VER
    {
        Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Single, Insight::Core::MemoryAllocCategory::Unknown);
        return ptr;
    }
    return nullptr;
}

void* operator new[](size_t size, std::align_val_t al)
{
    if (size == 0)
    {
        return nullptr;
    }

#if _MSC_VER
    if (void* ptr = _aligned_malloc(size, static_cast<size_t>(al)))
#else
    assert((size % al) == 0);
    if (void* ptr = std::aligned_alloc(size))
#endif // _MSC_VER
    {
        Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Array, Insight::Core::MemoryAllocCategory::Unknown);
        return ptr;
    }
    return nullptr;
}

void operator delete(void* ptr, std::align_val_t al)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

void operator delete(void* ptr, u64 bytes, std::align_val_t al)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

void operator delete[](void* ptr, std::align_val_t al)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}

void operator delete[](void* ptr, u64 bytes, std::align_val_t al)
{
    Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
#if _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif // _MSC_VER
}
#endif // C_PLUS_PLUS_STANDARD >= 201703L

#endif