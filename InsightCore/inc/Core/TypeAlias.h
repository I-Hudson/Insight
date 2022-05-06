#pragma once

#include <cstdint>
#include <type_traits>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using Byte = unsigned char;

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
	T r = x % align;
	return r ? x + (align - r) : x;
}