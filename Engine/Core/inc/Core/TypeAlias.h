#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;

using i8    = int8_t;
using i16   = int16_t;
using i32   = int32_t;
using i64   = int64_t;

using Byte = unsigned char;
using IntPtr = intptr_t;

using Handle = u64;

constexpr inline auto operator""_B(u64 const x) { return x; }
constexpr inline auto operator""_KB(u64 const x) { return 1024 * x; }
constexpr inline auto operator""_MB(u64 const x) { return 1024 * 1024 * x; }
constexpr inline auto operator""_GB(u64 const x) { return 1024 * 1024 * 1024 * x; }

#ifdef IS_PLATFORM_X64
constexpr int log2_64(u64 value);
constexpr int log2(u64 value) { return log2_64(value); }
#elif IS_PLATFORM_X32
constexpr int log2_32(u32 value);
constexpr int log2(u64 value) { return log2_32((u32)value); }
#endif

template <class Seed, class T>
constexpr void HashCombine(Seed& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T>
constexpr T AlignUp(T x, T align)
{
	static_assert((std::is_integral_v<T>));
    if (align == 0)
    {
        return x;
    }
	return (x + align - 1) / align * align;
}

template<typename T,typename Y>
constexpr T AlignUp(T x, Y align)
{
    static_assert((std::is_integral_v<T>));
    static_assert((std::is_integral_v<Y>));

    const u64 xValue = static_cast<u64>(x);
    const u64 alignValue = static_cast<u64>(align);

    if (alignValue == 0)
    {
        return x;
    }
    return static_cast<T>((xValue + alignValue - 1ull) / alignValue * alignValue);
}

/// https:///stackoverflow.com/a/21624122
constexpr int BitFlagsToIndex(u64 flags)
{
	int index = static_cast<int>(log2(flags - (flags & flags - 1)));
	return index;
}

const int log2Tab64[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5 };

constexpr int log2_64(u64 value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return log2Tab64[((u64)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
}

const int tab32[32] = {
     0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
     8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31 };

constexpr int log2_32(u32 value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(u32)(value * 0x07C4ACDD) >> 27];
}

template<typename T>
constexpr T* RemoveConst(const T* value)
{
    return const_cast<T*>(value);
}
template<typename T>
constexpr T& RemoveConst(const T& value)
{
    return const_cast<T&>(value);
}