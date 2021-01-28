#pragma once

#include "Engine/Core/Compiler.h"
#include "Engine/Core/InsightAlias.h"
#include "Engine/Core/Templates.h"

inline U32 GetHash(const U8 key)
{
    return key;
}

inline U32 GetHash(const I8 key)
{
    return key;
}

inline U32 GetHash(const U16 key)
{
    return key;
}

inline U32 GetHash(const I16 key)
{
    return key;
}

inline U32 GetHash(const I32 key)
{
    return key;
}

inline U32 GetHash(const U32 key)
{
    return key;
}

inline U32 GetHash(const U64 key)
{
    return (U32)key + ((U32)(key >> 32) * 23);
}

inline U32 GetHash(const I64 key)
{
    return (U32)key + ((U32)(key >> 32) * 23);
}

inline U32 GetHash(const char key)
{
    return key;
}

inline U32 GetHash(const char key)
{
    return key;
}

inline U32 GetHash(const float key)
{
    return *(U32*)&key;
}

inline U32 GetHash(const double key)
{
    return GetHash(*(U64*)&key);
}

inline U32 GetHash(const void* key)
{
    static const I64 shift = 3;
    return (U32)((I64)(key) >> shift);
}

inline U32 GetHash(void* key)
{
    static const I64 shift = 3;
    return (U32)((I64)(key) >> shift);
}

template<typename EnumType>
inline typename TEnableIf<TIsEnum<EnumType>::Value, U32>::Type GetHash(const EnumType key)
{
    return GetHash((__underlying_type(EnumType))key);
}

inline void CombineHash(U32& hash, const U32 value)
{
    // Reference: Boost lib
    hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

template<typename T>
inline void CombineHash(U32& hash, const T* value)
{
    CombineHash(hash, GetHash(value));
}
