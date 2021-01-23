#pragma once

#include <memory>
#include <functional>
#include <mutex>

using Byte = uint8_t;
using Size = size_t;

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;
using U128 = _GUID;

constexpr U8 U8_MAX = 0xFF;
constexpr U16 U16_MAX = 0xFFFF;
constexpr U32 U32_MAX = 0xFFFFFFFF;
constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFF;

using I8 = int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;

using WChar = wchar_t;

using PtrDiff = std::ptrdiff_t;
using PtrInt = uintptr_t;

template <typename... T>
using Action = std::function<void(T...)>;

template <typename result, typename... T>
using Func = std::function<result(T...)>;

using MutexUnqiueLock = std::unique_lock<std::mutex>;
using MutexLockGuard = std::lock_guard<std::mutex>;
using MutexScopedLock = std::scoped_lock<std::mutex>;

inline auto operator""_B(Size const x) { return x; }
inline auto operator""_KB(Size const x) { return 1024 * x; }
inline auto operator""_MB(Size const x) { return 1024 * 1024 * x; }
inline auto operator""_GB(Size const x) { return 1024 * 1024 * 1024 * x; }

#if defined(IS_STANDARD_POINTER)
template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;
#else
#error #define 'IS_STANDARD_POINTER''

//TODO: Finish this
template<typename T>
class TUniquePtr<T>;
using UniquePtr = TUniquePtr<T>;
#endif

// Declares full set of operators for the enum type (using binary operation on integer values)
#define DECLARE_ENUM_OPERATORS(T) \
    inline T operator~ (T a) { return (T)~(int)a; } \
    inline T operator| (T a, T b) { return (T)((int)a | (int)b); } \
    inline int operator& (T a, T b) { return ((int)a & (int)b); } \
    inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); } \
    inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); } \
    inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); } \
    inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }
