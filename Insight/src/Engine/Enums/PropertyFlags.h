#pragma once
#include "Engine/Core/Core.h"

typedef uint32_t PropertyFlags;

#define PropertyFlagEditor 1
#define PropertyFlagGeneral 8

namespace
{
	enum IS_PropertyFlags : U32
	{
		None = 1 << 0,

		// Editor
		ShowInEditor = 1 << PropertyFlagEditor,
		ReadOnlyEditor = 1 << (PropertyFlagEditor + 1),
		TextBold = 1 << (PropertyFlagEditor + 2),

		// General
		ClampZero = 1 << PropertyFlagGeneral,
		ReadOnly = 1 << (PropertyFlagGeneral + 1),
	};
}