#pragma once

#include "Core/Defines.h"

namespace Insight
{
	namespace Core
	{
		void IS_CORE WaitForProfiler();
	}
}

#if defined(IS_PROFILE_ENABLED)
#if defined(IS_PROFILE_OPTICK)
	#include "optick.h"
	#define IS_PROFILE_FUNCTION()			OPTICK_EVENT()
	#define IS_PROFILE_SCOPE(name)			OPTICK_EVENT(name)
	#define IS_PROFILE_FRAME(frame_name)	OPTICK_FRAME(frame_name)
#elif defined(IS_PROFILE_TRACY)
#define TRACY_ENABLE
	#include "public/tracy/Tracy.hpp"
	#define IS_PROFILE_FUNCTION()			ZoneScoped
	#define IS_PROFILE_SCOPE(name)			ZoneScopedN(name)
	#define IS_PROFILE_FRAME(frame_name)	FrameMarkNamed(frame_name)
#endif
#else
	#define IS_PROFILE_FUNCTION()		
	#define IS_PROFILE_SCOPE(name)		
	#define IS_PROFILE_FRAME(frame_name)
#endif
