#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

namespace Insight::Core
{
	enum class MemoryAllocCategory : u8
	{
		General,
		Threading,
		Core,
		Maths,
		Input,
		Graphics,
		Resources,
		ECS,
		World,
		Serialiser,
		Editor,

		Unknown,

		Size
	};
	constexpr const char* MemoryAllocCategoryToString[] =
	{
		"General",
		"Threading",
		"Core",
		"Maths",
		"Input",
		"Graphics",
		"Resources",
		"ECS",
		"World",
		"Serialiser",
		"Editor",
		"Unknown",
	};
	static_assert(ARRAY_COUNT(MemoryAllocCategoryToString) == static_cast<u64>(MemoryAllocCategory::Size));
}