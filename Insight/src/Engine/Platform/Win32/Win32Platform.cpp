#include "ispch.h"
#include "Engine/Platform/Win32/Win32Platform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Memory/ProfilerMemory.h"

#include <chrono>
#include <thread>
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

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

namespace
{
    u64 ClockFrequency;
    double CyclesToSeconds;
}

void Win32Platform::Init()
{
    // Init timing
    LARGE_INTEGER frequency;
    const auto freqResult = QueryPerformanceFrequency(&frequency);
    ASSERT(freqResult && frequency.QuadPart > 0);
    ClockFrequency = frequency.QuadPart;
    CyclesToSeconds = 1.0 / static_cast<double>(frequency.QuadPart);
}

void Win32Platform::MemoryBarrier()
{
}

i64 Win32Platform::InterlockedExchange(i64 volatile* dst, i64 exchange)
{
    return InterlockedExchange64(dst, exchange);
}

i32 Win32Platform::InterlockedCompareExchange(i32 volatile* dst, i32 exchange, i32 comperand)
{
    static_assert(sizeof(i32) == sizeof(LONG), "Invalid LONG size.");
    return _InterlockedCompareExchange((LONG volatile*)dst, exchange, comperand);
}

i64 Win32Platform::InterlockedCompareExchange(i64 volatile* dst, i64 exchange, i64 comperand)
{
    return InterlockedCompareExchange64(dst, exchange, comperand);
}

i64 Win32Platform::InterlockedIncrement(i64 volatile* dst)
{
    return InterlockedIncrement64(dst);
}

i64 Win32Platform::InterlockedDecrement(i64 volatile* dst)
{
    return InterlockedDecrement64(dst);
}

i64 Win32Platform::InterlockedAdd(i64 volatile* dst, i64 value)
{
    return InterlockedExchangeAdd64(dst, value);
}

i32 Win32Platform::AtomicRead(i32 volatile* dst)
{
    static_assert(sizeof(i32) == sizeof(LONG), "Invalid LONG size.");
    return _InterlockedCompareExchange((LONG volatile*)dst, 0, 0);
}

i64 Win32Platform::AtomicRead(i64 volatile* dst)
{
    return InterlockedCompareExchange64(dst, 0, 0);
}

void Win32Platform::AtomicStore(i32 volatile* dst, i32 value)
{
    static_assert(sizeof(i32) == sizeof(LONG), "Invalid LONG size.");
    _InterlockedExchange((LONG volatile*)dst, value);
}

void Win32Platform::AtomicStore(i64 volatile* dst, i64 value)
{
    InterlockedExchange64(dst, value);
}

void Win32Platform::TrackAllocation(void* ptr, u64 size)
{
#ifdef IS_DEBUG
    Insight::Core::ProfilerMemory::Instance()->TrackAllocation(ptr, size);
#endif
}

void Win32Platform::UnTrackAllocation(void* ptr)
{
#ifdef IS_DEBUG
    Insight::Core::ProfilerMemory::Instance()->UnTrackAllocation(ptr);
#endif
}

void Win32Platform::MemCopy(void* dst, void const* src, const u64& size)
{
    memcpy(dst, src, size);
}

void Win32Platform::MemSet(void* dst, const u32& value, const u64& size)
{
    memset(dst, value, size);
}

void Win32Platform::MemClear(void* dst, const u64& size)
{
    memset(dst, 0, size);
}

bool Win32Platform::MemCompare(const void* buf1, const void* buf2, const u64& size)
{
    return memcmp(buf1, buf2, static_cast<size_t>(size));
}

void Win32Platform::Assert(const char* message, const char* file, int line)
{
    IS_CORE_FATEL("Assertion failed!\nFile: {0}\nLine : {1}\n\nExpression: {2}", file, line, message);
    PLATFORM_DEBUG_BREAK;
}

void Win32Platform::CheckFailed(const char* message, const char* file, int line)
{
    IS_FATEL("Check failed!\nFile: {0}\nLine: {1}\n\nExpression: {2}", file, line, message);
}

void Win32Platform::Sleep(float milliseconds)
{
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(milliseconds));
}

double Win32Platform::GetTimeSeconds()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return double(counter.QuadPart) * CyclesToSeconds;
}

u128 Win32Platform::GenerateUUID()
{
    u128 uuid;
    RPC_STATUS ret = UuidCreate(&uuid);
    ASSERT(ret == RPC_S_OK);
    return uuid;
}

std::string Win32Platform::UUIDToString(const u128& uuid)
{
    std::string str;

    // convert UUID to LPWSTR
    WCHAR* wszUuid = NULL;
    ::UuidToStringW(&uuid, (RPC_WSTR*)&wszUuid);
    if (wszUuid != NULL)
    {
        //TODO: do something with wszUuid
        str = WStringToString(wszUuid);

        // free up the allocated string
        ::RpcStringFreeW((RPC_WSTR*)&wszUuid);
        wszUuid = NULL;
    }

    return str;
}

void Win32Platform::GetSystemTime(i32& year, i32& month, i32& dayOfWeek, i32& day, i32& hour, i32& minute, i32& second, i32& millisecond)
{
    // Get current local time
    SYSTEMTIME st;
    ::GetLocalTime(&st);

    // Extract time
    year = st.wYear;
    month = st.wMonth;
    dayOfWeek = st.wDayOfWeek;
    day = st.wDay;
    hour = st.wHour;
    minute = st.wMinute;
    second = st.wSecond;
    millisecond = st.wMilliseconds;
}

void Win32Platform::GetUTCTime(i32& year, i32& month, i32& dayOfWeek, i32& day, i32& hour, i32& minute, i32& second, i32& millisecond)
{
    // Get current system time
    SYSTEMTIME st;
    ::GetSystemTime(&st);

    // Extract time
    year = st.wYear;
    month = st.wMonth;
    dayOfWeek = st.wDayOfWeek;
    day = st.wDay;
    hour = st.wHour;
    minute = st.wMinute;
    second = st.wSecond;
    millisecond = st.wMilliseconds;
}