#pragma once 

#include "Engine/Platform/Win32/Win32Platform.h"

class WindowsPlatform : public Win32Platform
{
public:
	static bool IsDebuggerPresent();
};