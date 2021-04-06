#pragma once

#include "Object.h"
#include "Compiler.h"
#include <string>

#if defined(IS_STANDARD_POINTER)

template<typename T, typename ... Args>
DEPRECATED constexpr UniquePtr<T> CreateUniquePtr(Args&& ... args)
{
	static_assert(!std::is_base_of_v<Object, T>, "'Object::CreateObjbect<T>' must be used.");
	return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename T, typename ... Args>
DEPRECATED constexpr UniquePtr<T> CreateUniquePtrNoExpect(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args>
DEPRECATED constexpr SharedPtr<T> CreateSharedPtr(Args&& ... args)
{
	static_assert(!std::is_base_of_v<Object, T>, "'Object::CreateObjbect<T>' must be used.");
	return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T, typename ... Args>
DEPRECATED constexpr SharedPtr<T> CreateSharedPtrNoExpect(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <class T, class T2>
DEPRECATED _NODISCARD SharedPtr<T> StaticPointerCast(const SharedPtr<T2>& other) noexcept
{
	return std::static_pointer_cast<T>(other);
}

template <class T, class T2>
DEPRECATED _NODISCARD SharedPtr<T> StaticPointerCast(SharedPtr<T2>&& other) noexcept
{
	return std::static_pointer_cast<T>(other);
}

template <class T, class T2>
DEPRECATED _NODISCARD SharedPtr<T> DynamicPointerCast(const SharedPtr<T2>& other) noexcept
{
	return std::dynamic_pointer_cast<T>(other);
}

template <class T, class T2>
DEPRECATED _NODISCARD SharedPtr<T> DynamicPointerCast(SharedPtr<T2>&& other) noexcept
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
    static const char* sizes[] = { "B", "KB", "MB", "GB", "TB" };
    U64 i = 0;
    double dblSByte = static_cast<double>(bytes);
    for (; static_cast<U64>(bytes / 1024.0) > 0; i++, bytes /= 1024)
        dblSByte = bytes / 1024.0;
    if (i >= ARRAY_COUNT(sizes))
        return "";
    return StringFormat("{0} {1}", RoundTo2DecimalPlaces(dblSByte), sizes[i]);
}

/// <summary>
/// Aligns value up to match desire alignment.
/// </summary>
/// <param name="value">The value.</param>
/// <param name="alignment">The alignment.</param>
/// <returns>Aligned value (multiple of alignment).</returns>
template<typename T>
FORCE_INLINE T AlignUp(T value, T alignment)
{
    T mask = alignment - 1;
    return (T)(value + mask & ~mask);
}

template<typename T>
FORCE_INLINE bool VectorContains(const std::vector<T>& vector, const T& item)
{
    return std::find(vector.begin(), vector.end(), item) != vector.end();
}

template<typename T>
FORCE_INLINE U32 VectorFindIndex(const std::vector<T>& vector, const T& item)
{
    for (size_t i = 0; i < vector.size(); ++i)
    {
        if (vector[i] == item)
        {
            return (U32)i;
        }
    }
    ASSERT(false && "VectorFindIndex. Out of range.");
    return ~0u;
}

INLINE I32 HexDigit(char c)
{
    I32 result = 0;

    if (c >= '0' && c <= '9')
    {
        result = c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        result = c + 10 - 'a';
    }
    else if (c >= 'A' && c <= 'F')
    {
        result = c + 10 - 'A';
    }
    else
    {
        result = 0;
    }

    return result;
}

#if defined _CONCAT
#undef _CONCAT
#endif
#define _CONCAT_EXPANDED(A,B) A##B
#define _CONCAT(A,B) _CONCAT_EXPANDED(A,B)