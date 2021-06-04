#pragma once

#include "Engine/Core/Compiler.h"
#include "Engine/Core/InsightAlias.h"
#include "Engine/Platform/Types.h"
#include "WindowsMinimal.h"

// Remove some Windows definitions
#undef MemoryBarrier
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef CreateDirectory
#undef GetComputerName
#undef GetUserName
#undef MessageBox
#undef GetCommandLine
#undef CreateWindow
#undef SetWindowText
#undef DrawText
#undef CreateFont
#undef IsMinimized
#undef IsMaximized
#undef LoadIcon
#undef InterlockedOr
#undef InterlockedAnd
#undef InterlockedExchange
#undef InterlockedCompareExchange
#undef InterlockedIncrement
#undef InterlockedDecrement
#undef InterlockedAdd
#undef GetObject
#undef GetClassName
#undef GetMessage
#undef CreateMutex
#undef DrawState
#undef LoadLibrary
#undef GetEnvironmentVariable
#undef SetEnvironmentVariable
#undef Rectangle

class Win32Platform
{
public:
    static void Init();

    static void MemoryBarrier();
    static i64 InterlockedExchange(i64 volatile* dst, i64 exchange);
    static i32 InterlockedCompareExchange(i32 volatile* dst, i32 exchange, i32 comperand);
    static i64 InterlockedCompareExchange(i64 volatile* dst, i64 exchange, i64 comperand);
    static i64 InterlockedIncrement(i64 volatile* dst);
    static i64 InterlockedDecrement(i64 volatile* dst);
    static i64 InterlockedAdd(i64 volatile* dst, i64 value);
    static i32 AtomicRead(i32 volatile* dst);
    static i64 AtomicRead(i64 volatile* dst);
    static void AtomicStore(i32 volatile* dst, i32 value);
    static void AtomicStore(i64 volatile* dst, i64 value);

    static void TrackAllocation(void* ptr, u64 size);
    static void UnTrackAllocation(void* ptr);

    FORCE_INLINE static void* Allocate(u64 size, u64 alignment)
    {
        void* ptr = _aligned_malloc((size_t)size, (size_t)alignment);
#if IS_DEBUG
        TrackAllocation(ptr, size);
#endif
        return ptr;
    }
    FORCE_INLINE static void Free(void* ptr)
    {
#if IS_DEBUG
        UnTrackAllocation(ptr);
#endif
        _aligned_free(ptr);
    }

	static void MemCopy(void* dst, void const* src, const u64& size);
	static void MemSet(void* dst, const u32& value, const u64& size);
	static void MemClear(void* dst, const u64& size);
    static bool MemCompare(const void* buf1, const void* buf2, const u64& size);

    static void Assert(const char* message, const char* file, int line);
    static void CheckFailed(const char* message, const char* file, int line);

    static void Sleep(i32 milliseconds);
    static double GetTimeSeconds();

	static U128 GenerateUUID();
	static std::string UUIDToString(const u128& uuid);

	static void GetSystemTime(i32& year, i32& month, i32& dayOfWeek, i32& day, i32& hour, i32& minute, i32& second, i32& millisecond);
	static void GetUTCTime(i32& year, i32& month, i32& dayOfWeek, i32& day, i32& hour, i32& minute, i32& second, i32& millisecond);

};