#include "ispch.h"

#include "Engine/Platform/Windows/PlatformWindows.h"
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

bool WindowsPlatform::IsDebuggerPresent()
{
    return !!::IsDebuggerPresent();
}
