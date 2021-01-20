#include "Core/Core.h"

class WindowsPlatform
{
public:
	static void MemCopy(void* dst, void* src, const U64& size);
	static void MemSet(void* dst, const U64& size);

	static U128 GenerateUUID();
	static std::string UUIDToString(const U128& uuid);

	static void GetSystemTime(I32& year, I32& month, I32& dayOfWeek, I32& day, I32& hour, I32& minute, I32& second, I32& millisecond);
	static void GetUTCTime(I32& year, I32& month, I32& dayOfWeek, I32& day, I32& hour, I32& minute, I32& second, I32& millisecond);
};