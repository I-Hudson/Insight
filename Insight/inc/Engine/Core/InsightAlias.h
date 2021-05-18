#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <bitset>

using Byte = uint8_t;
using Size = size_t;

using U8 = uint8_t; // TODO: Make deprecated 
using U16 = uint16_t; // TODO: Make deprecated 
using U128 = _GUID; // TODO: Make deprecated 

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = _GUID;

inline constexpr u8 U8_MAX = 0xFF;
inline constexpr u16 U16_MAX = 0xFFFF;
inline constexpr u32 U32_MAX = 0xFFFFFFFF;
inline constexpr u64 U64_MAX = 0xFFFFFFFFFFFFFFFF;

inline constexpr float FLOAT_MIN = 1.175494e-38f;
inline constexpr float FLOAT_MAX = 3.402823e+38f;
inline constexpr float FLOAT_EPSILON = 1.192093e-07f;

using I8 = int8_t; // TODO: Make deprecated 
using I16 = int16_t; // TODO: Make deprecated 
using I32 = int32_t; // TODO: Make deprecated 
using I64 = int64_t; // TODO: Make deprecated 

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

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

const u32 MAX_COMPONENTS_COUNT = 32;
const u32 MAX_ENTITIES_COUNT = 5000;

using EntityID = u32;
using ComponentID = u32;
using EntityComponentSignature = std::bitset<MAX_COMPONENTS_COUNT>;
using ComponentType = u32;

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
