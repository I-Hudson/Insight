#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

namespace Insight::Core
{
	enum class MemoryAllocCategory : u8
	{
		General,
		Threading,
		Maths,
		Input,
		Graphics,
		Resources,
		ECS,
		World,
		Serialiser,
		Editor,

		Size
	};
	constexpr const char* MemoryAllocCategoryToString[] =
	{
		"General",
		"Threading",
		"Maths",
		"Input",
		"Graphics",
		"Resources",
		"ECS",
		"World",
		"Serialiser",
		"Editor",
	};
	static_assert(ARRAY_COUNT(MemoryAllocCategoryToString) == static_cast<u64>(MemoryAllocCategory::Size));
}