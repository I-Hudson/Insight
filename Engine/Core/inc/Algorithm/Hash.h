#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

#include <string>

namespace Insight
{
	namespace Algorithm
	{
        IS_CORE FORCE_INLINE u32 GetHash32(const char* cStr, u64 length);
        IS_CORE FORCE_INLINE u32 GetHash32(const char* cStr);
        IS_CORE FORCE_INLINE u32 GetHash32(const std::string& str);

        IS_CORE FORCE_INLINE u64 GetHash64(const char* cStr, u64 length);
        IS_CORE FORCE_INLINE u64 GetHash64(const char* cStr);
        IS_CORE FORCE_INLINE u64 GetHash64(const std::string& str);

        namespace FNV1a
        {
            CONSTEXPR u32 const g_constValue32 = 0x811c9dc5;
            CONSTEXPR u32 const g_defaultOffsetBasis32 = 0x1000193;
            CONSTEXPR u64 const g_constValue64 = 0xcbf29ce484222325;
            CONSTEXPR u64 const g_defaultOffsetBasis64 = 0x100000001b3;

            CONSTEXPR static INLINE u32 GetHash32(char const* const str, const u32 val = g_constValue32)
            {
                return (str[0] == '\0') ? val : GetHash32(&str[1], ((uint64_t)val ^ uint32_t(str[0])) * g_defaultOffsetBasis32);
            }
            CONSTEXPR static INLINE u64 GetHash64(char const* const str, const u64 val = g_constValue64)
            {
                return (str[0] == '\0') ? val : GetHash64(&str[1], ((uint64_t)val ^ uint64_t(str[0])) * g_defaultOffsetBasis64);
            }
        }
	}
}