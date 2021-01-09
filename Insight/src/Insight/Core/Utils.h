#pragma once

#include "Object.h"
#include <string>

#if defined(IS_STANDARD_POINTER)

template<typename T, typename ... Args>
constexpr UniquePtr<T> CreateUniquePtr(Args&& ... args)
{
	static_assert(!std::is_base_of_v<Insight::Object, T>, "'Object::CreateObjbect<T>' must be used.");
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
	static_assert(!std::is_base_of_v<Insight::Object, T>, "'Object::CreateObjbect<T>' must be used.");
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

inline std::string CheckAndAppend(const char* extension, const std::string& str)
{
	if (str.find(extension) == std::string::npos)
	{
		return std::string(str).append(extension);
	}
	return str;
}

inline std::wstring StringToWString(const char* str)
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

inline std::string WStringToString(const WCHAR* str, int cbStr = -1)
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