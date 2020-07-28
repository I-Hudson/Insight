#pragma once

#include "ispch.h"

#include <memory>

namespace Insight
{
	using Byte = uint8_t;
	using Size = size_t;

	using U8 = uint8_t;
	using U16 = uint16_t;
	using U32 = uint32_t;
	using U64 = uint64_t;

	constexpr U8 U8_MAX = 0xFF;
	constexpr U16 U16_MAX = 0xFFFF;
	constexpr U32 U32_MAX = 0xFFFFFFFF;
	constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFF;

	using I8 = int8_t;
	using I16 = int16_t;
	using I32 = int32_t;
	using I64 = int64_t;

	using PtrDiff = std::ptrdiff_t;
	using PtrInt = uintptr_t;

	template <typename... T>
	using Action = std::function<void(T...)>;

	template <typename result, typename... T>
	using Func = std::function<result(T...)>;

	inline auto operator""_B(Size const x) { return x; }
	inline auto operator""_KB(Size const x) { return 1024 * x; }
	inline auto operator""_MB(Size const x) { return 1024 * 1024 * x; }
	inline auto operator""_GB(Size const x) { return 1024 * 1024 * 1024 * x; }

}

	template<typename T>
	using UniquePtr = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr UniquePtr<T> CreateUniquePtr(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using SharedPtr = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr SharedPtr<T> CreateSharedPtr(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
