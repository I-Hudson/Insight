#include "ispch.h"

#include "PlatformWindows.h"
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

void WindowsPlatform::MemCopy(void* dst, void* src, const U64& size)
{
	memcpy(dst, src, size);
}

void WindowsPlatform::MemSet(void* dst, const U64& size)
{
	memset(dst, 0, size);
}

U128 WindowsPlatform::GenerateUUID()
{
    U128 uuid;
    RPC_STATUS ret = UuidCreate(&uuid);
    IS_CORE_ASSERT(ret == RPC_S_OK, "[WindowsPlatform::GenerateUUID] UUID generated must be vaild.");
    return uuid;
}

std::string WindowsPlatform::UUIDToString(const U128& uuid)
{
    std::string str;

    // convert UUID to LPWSTR
    WCHAR* wszUuid = NULL;
    ::UuidToStringW(&uuid, (RPC_WSTR*)&wszUuid);
    if (wszUuid != NULL)
    {
        //TODO: do something with wszUuid
        str = WStringToString(wszUuid);

        // free up the allocated string
        ::RpcStringFreeW((RPC_WSTR*)&wszUuid);
        wszUuid = NULL;
    }

    return str;
}

void WindowsPlatform::GetSystemTime(I32& year, I32& month, I32& dayOfWeek, I32& day, I32& hour, I32& minute, I32& second, I32& millisecond)
{
    // Get current local time
    SYSTEMTIME st;
    ::GetLocalTime(&st);

    // Extract time
    year = st.wYear;
    month = st.wMonth;
    dayOfWeek = st.wDayOfWeek;
    day = st.wDay;
    hour = st.wHour;
    minute = st.wMinute;
    second = st.wSecond;
    millisecond = st.wMilliseconds;
}

void WindowsPlatform::GetUTCTime(I32& year, I32& month, I32& dayOfWeek, I32& day, I32& hour, I32& minute, I32& second, I32& millisecond)
{
    // Get current system time
    SYSTEMTIME st;
    ::GetSystemTime(&st);

    // Extract time
    year = st.wYear;
    month = st.wMonth;
    dayOfWeek = st.wDayOfWeek;
    day = st.wDay;
    hour = st.wHour;
    minute = st.wMinute;
    second = st.wSecond;
    millisecond = st.wMilliseconds;
}
