#pragma once

#include "Object.h"
#include "Compiler.h"
#include <string>

#if defined(IS_STANDARD_POINTER)

template<typename T, typename ... Args>
constexpr UniquePtr<T> CreateUniquePtr(Args&& ... args)
{
	static_assert(!std::is_base_of_v<Object, T>, "'Object::CreateObjbect<T>' must be used.");
	return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename T, typename ... Args>
constexpr UniquePtr<T> CreateUniquePtrNoExpect(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args>
constexpr SharedPtr<T> CreateSharedPtr(Args&& ... args)
{
	static_assert(!std::is_base_of_v<Object, T>, "'Object::CreateObjbect<T>' must be used.");
	return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T, typename ... Args>
constexpr SharedPtr<T> CreateSharedPtrNoExpect(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <class T, class T2>
_NODISCARD SharedPtr<T> StaticPointerCast(const SharedPtr<T2>& other) noexcept
{
	return std::static_pointer_cast<T>(other);
}

template <class T, class T2>
_NODISCARD SharedPtr<T> StaticPointerCast(SharedPtr<T2>&& other) noexcept
{
	return std::static_pointer_cast<T>(other);
}

template <class T, class T2>
_NODISCARD SharedPtr<T> DynamicPointerCast(const SharedPtr<T2>& other) noexcept
{
	return std::dynamic_pointer_cast<T>(other);
}

template <class T, class T2>
_NODISCARD SharedPtr<T> DynamicPointerCast(SharedPtr<T2>&& other) noexcept
{
	return std::dynamic_pointer_cast<T>(other);
}
#else
#error #define 'IS_STANDARD_POINTER''

//TODO: Finish this
template<typename T>
class TUniquePtr<T>;
using UniquePtr = TUniquePtr<T>;
#endif

INLINE std::string CheckAndAppend(const char* extension, const std::string& str)
{
	if (str.find(extension) == std::string::npos)
	{
		return std::string(str).append(extension);
	}
	return str;
}

INLINE std::wstring StringToWString(const char* str)
{
    if (!str)
    {
        return std::wstring();
    }

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    std::wstring unicodeStr(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &unicodeStr[0], sizeNeeded);
    return unicodeStr;
}

INLINE std::string WStringToString(const WCHAR* str, int cbStr = -1)
{
    if (!str)
    {
        return std::string();
    }

    int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, str, cbStr, nullptr, 0, nullptr, nullptr);
    std::string ansiStr(sizeNeeded, 0);
    WideCharToMultiByte(CP_ACP, 0, str, cbStr, &ansiStr[0], sizeNeeded, nullptr, nullptr);
    return ansiStr;
}

template<typename... Args>
INLINE std::string FormatString(const std::string& format, Args... args)
{
    const auto size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    const auto buffer = std::make_unique<char[]>(size);

    std::snprintf(buffer.get(), size, format.c_str(), args...);

    return std::string(buffer.get(), buffer.get() + size - 1);
}

// Round floating point value up to 1 decimal place
template<typename T>
INLINE T RoundTo1DecimalPlace(T value)
{
    return round(value * 10) / 10;
}

// Round floating point value up to 2 decimal places
template<typename T>
FORCE_INLINE T RoundTo2DecimalPlaces(T value)
{
    return round(value * 100) / 100;
}

// Round floating point value up to 3 decimal places
template<typename T>
FORCE_INLINE T RoundTo3DecimalPlaces(T value)
{
    return round(value * 1000) / 1000;
}

// Converts size of the file (in bytes) to the best fitting string
// @param bytes Size of the file in bytes
// @return The best fitting string of the file size
template<typename T>
std::string BytesToText(T bytes)
{
    static const char* sizes[] = { TEXT("B"), TEXT("KB"), TEXT("MB"), TEXT("GB"), TEXT("TB") };
    U64 i = 0;
    double dblSByte = static_cast<double>(bytes);
    for (; static_cast<U64>(bytes / 1024.0) > 0; i++, bytes /= 1024)
        dblSByte = bytes / 1024.0;
    if (i >= ARRAY_COUNT(sizes))
        return "";
    return StringFormat("{0} {1}", RoundTo2DecimalPlaces(dblSByte), sizes[i]);
}

#if defined _CONCAT
#undef _CONCAT
#endif
#define _CONCAT_EXPANDED(A,B) A##B
#define _CONCAT(A,B) _CONCAT_EXPANDED(A,B)