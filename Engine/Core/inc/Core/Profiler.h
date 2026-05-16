#pragma once

#include "Core/Defines.h"

namespace Insight
{
	namespace Core
	{
		/// Wait for the profiler to be a attached.
		void IS_CORE WaitForProfiler();
		bool IS_CORE IsProfilerAttached();
		void IS_CORE ShutdownProfiler();
	}
}

#if defined(IS_PROFILE_ENABLED)
#if defined(IS_PROFILE_OPTICK)
	#include "optick.h"
	#define IS_PROFILE_FUNCTION()			OPTICK_EVENT()
	#define IS_PROFILE_SCOPE(name)			OPTICK_EVENT(name)
	#define IS_PROFILE_FRAME(frame_name)	OPTICK_FRAME(frame_name)
	#define IS_PROFILE_THREAD(name)			OPTICK_THREAD(name)
	#define IS_PROFILE_SCOPE_TEXT(format, ...)

#elif defined(IS_PROFILE_TRACY)
#ifndef TRACY_ENABLE
#define TRACY_ENABLE 1
#endif // TRACY_ENABLE
#ifndef TRACY_ON_DEMAND
#define TRACY_ON_DEMAND
#endif // TRACY_ON_DEMAND
	#include "public/tracy/Tracy.hpp"
	#define IS_PROFILE_FUNCTION()			ZoneScoped
	#define IS_PROFILE_SCOPE(name)			ZoneScopedN(name)
	#define IS_PROFILE_FRAME(frame_name)	FrameMarkNamed(frame_name)
	#define IS_PROFILE_THREAD(name)			::tracy::SetThreadName(name);
	#define IS_PROFILE_SCOPE_TEXT(format, ...)	ZoneTextF(format, __VA_ARGS__)

#elif defined(IS_PROFILE_PIX)
#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include "WinPixEventRuntime/pix3.h"
	#define IS_PROFILE_FUNCTION()			PIXScopedEvent(PIX_COLOR_DEFAULT, __FUNCTION__)
	#define IS_PROFILE_SCOPE(name)			PIXScopedEvent(PIX_COLOR_DEFAULT, name)
	#define IS_PROFILE_FRAME(frame_name)	PIXScopedEvent(PIX_COLOR_DEFAULT, frame_name)
	#define IS_PROFILE_THREAD(name)
	#define IS_PROFILE_SCOPE_TEXT(format, ...)

#else
	#define IS_PROFILE_FUNCTION()		
	#define IS_PROFILE_SCOPE(name)		
	#define IS_PROFILE_FRAME(frame_name)
	#define IS_PROFILE_THREAD(name)
	#define IS_PROFILE_SCOPE_TEXT(format, ...)

#endif
#else
#define IS_PROFILE_FUNCTION()		
#define IS_PROFILE_SCOPE(name)		
#define IS_PROFILE_FRAME(frame_name)
#define IS_PROFILE_THREAD(name)
#define IS_PROFILE_SCOPE_TEXT(format, ...)
#endif