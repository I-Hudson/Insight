#include "ispch.h"

#include "PlatformWindows.h"
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

bool WindowsPlatform::IsDebuggerPresent()
{
    return !!::IsDebuggerPresent();
}
