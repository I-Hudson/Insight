#pragma once

#define IS_PROFILE_ENABLED

#if defined(IS_PROFILE_ENABLED)

#if defined(IS_PROFILE_OPTICK)
	#include "optick.h"
	#define IS_PROFILE_FUNCTION()			OPTICK_EVENT()
	#define IS_PROFILE_SCOPE(name)			OPTICK_EVENT(name)
	#define IS_PROFILE_FRAME(frame_name)	OPTICK_FRAME(frame_name)
#elif defined(IS_PROFILE_TRACY)
	#include "taccy.hpp"
	#define IS_PROFILE_FUNCTION()			OPTICK_EVENT()
	#define IS_PROFILE_SCOPE(name)			OPTICK_EVENT(name)
	#define IS_PROFILE_FRAME(frame_name)	OPTICK_FRAME(frame_name)
#endif
#else
	#define IS_PROFILE_FUNCTION()		
	#define IS_PROFILE_SCOPE(name)		
	#define IS_PROFILE_FRAME(frame_name)
#endif
