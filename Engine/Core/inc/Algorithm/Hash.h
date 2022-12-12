#pragma once

#include "Core/TypeAlias.h"

namespace Insight
{
	namespace Algorithm
	{
        namespace FNV1a
        {
            constexpr u32 const g_constValue32 = 0x811c9dc5;
            constexpr u32 const g_defaultOffsetBasis32 = 0x1000193;
            constexpr u64 const g_constValue64 = 0xcbf29ce484222325;
            constexpr u64 const g_defaultOffsetBasis64 = 0x100000001b3;

            constexpr static inline uint32_t GetHash32(char const* const str, const uint32_t val = g_constValue32)
            {
                return (str[0] == '\0') ? val : GetHash32(&str[1], ((uint64_t)val ^ uint32_t(str[0])) * g_defaultOffsetBasis32);
            }

            constexpr static inline uint64_t GetHash64(char const* const str, const uint64_t val = g_constValue64)
            {
                return (str[0] == '\0') ? val : GetHash64(&str[1], ((uint64_t)val ^ uint64_t(str[0])) * g_defaultOffsetBasis64);
            }
        }
	}
}