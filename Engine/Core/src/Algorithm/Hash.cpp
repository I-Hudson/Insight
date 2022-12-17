#include "Algorithm/Hash.h"

#include "xxhash.h"

namespace Insight
{
	namespace Algorithm
	{
		constexpr static uint32_t const g_hashSeed = 'EE8';


		FORCE_INLINE u32 GetHash32(const char* cStr, u64 length)
		{
			return XXH32(cStr, length, g_hashSeed);
		}
		FORCE_INLINE u32 GetHash32(const char* cStr)
		{
			return GetHash32(cStr, strlen(cStr));
		}
		FORCE_INLINE u32 GetHash32(const std::string& str)
		{
			return GetHash32(str.c_str(), str.size());
		}

		FORCE_INLINE u64 GetHash64(const char* cStr, u64 length)
		{
			return XXH64(cStr, length, g_hashSeed);
		}
		FORCE_INLINE u64 GetHash64(const char* cStr)
		{
			return GetHash64(cStr, strlen(cStr));
		}
		FORCE_INLINE u64 GetHash64(const std::string& str)
		{
			return GetHash64(str.c_str(), str.size());
		}
	}
}