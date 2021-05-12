#pragma once

#include "Engine/Core/Compiler.h"
#include "Engine/Core/InsightAlias.h"
#include "Engine/Core/Templates.h"

inline u32 GetHash(const U8 key)
{
    return key;
}

inline u32 GetHash(const I8 key)
{
    return key;
}

inline u32 GetHash(const U16 key)
{
    return key;
}

inline u32 GetHash(const I16 key)
{
    return key;
}

inline u32 GetHash(const I32 key)
{
    return key;
}

inline u32 GetHash(const u32 key)
{
    return key;
}

inline u32 GetHash(const u64 key)
{
    return (u32)key + ((u32)(key >> 32) * 23);
}

inline u32 GetHash(const I64 key)
{
    return (u32)key + ((u32)(key >> 32) * 23);
}

inline u32 GetHash(const char key)
{
    return key;
}

inline u32 GetHash(const WCHAR key)
{
    return key;
}

inline u32 GetHash(const float key)
{
    return *(u32*)&key;
}

inline u32 GetHash(const double key)
{
    return GetHash(*(u64*)&key);
}

inline u32 GetHash(const void* key)
{
    static const I64 shift = 3;
    return (u32)((I64)(key) >> shift);
}

inline u32 GetHash(void* key)
{
    static const I64 shift = 3;
    return (u32)((I64)(key) >> shift);
}

template<typename EnumType>
inline typename TEnableIf<TIsEnum<EnumType>::Value, u32>::Type GetHash(const EnumType key)
{
    return GetHash((__underlying_type(EnumType))key);
}

inline void CombineHash(u32& hash, const u32 value)
{
    // Reference: Boost lib
    hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

template<typename T>
inline void CombineHash(u32& hash, const T* value)
{
    CombineHash(hash, GetHash(value));
}

class Hasher
{
public:
    Hasher(u64 hash)
        : m_hash(hash)
    { }

    Hasher() {}

    template<typename T>
    INLINE void Data(const T* data, u64 size)
    {
        size /= sizeof(*data);
        for (u64 i = 0; i < size; ++i)
            m_hash = (m_hash * 0x100000001b3ull) ^ data[i];
    }

	INLINE void u32(uint32_t value)
	{
		m_hash = (m_hash * 0x100000001b3ull) ^ value;
	}

	INLINE void s32(int32_t value)
	{
		u32(uint32_t(value));
	}

	INLINE void f32(float value)
	{
		union
		{
			float f32;
			uint32_t u32;
		} u;
		u.f32 = value;
		u32(u.u32);
	}

	INLINE void u64(uint64_t value)
	{
		u32(value & 0xffffffffu);
		u32(value >> 32);
	}

	template <typename T>
	INLINE void Pointer(T* ptr)
	{
		u64(reinterpret_cast<uintptr_t>(ptr));
	}

	INLINE void String(const char* str)
	{
		char c;
		u32(0xff);
		while ((c = *str++) != '\0')
			u32(uint8_t(c));
	}

	INLINE void String(const std::string& str)
	{
		u32(0xff);
		for (auto& c : str)
			u32(uint8_t(c));
	}

	INLINE uint64_t Get() const
	{
		return m_hash;
	}

private:
	uint64_t m_hash = 0xcbf29ce484222325ull;
};
