#pragma once

#include "Graphics/Enums.h"

#include <string>

namespace Insight
{
	namespace Graphics
	{
#if 0
		template<typename E, E V>
		constexpr bool IsValid()
		{
			/// When compiled with clang, `name` will contain a prettified function name,
			/// including the enum value name for `V` if valid. For example:
			/// "bool IsValid() [E = Fruit, V = Fruit::BANANA]" for valid enum values, or:
			/// "bool IsValid() [E = Fruit, V = 10]" for invalid enum values.
			constexpr const char* name = FUNCTION;
			int i = strlen(name);
			/// Find the final space character in the pretty name.
			for (; i >= 0; --i)
			{
				if (name[i] == ' ')
				{
					break;
				}
			}
			/// The character after the final space will indicate if
			/// it's a valid value or not.
			char c = name[i + 1];
			if (c >= '0' && c <= '9')
			{
				return false;
			}
			return true;
		}

		template<typename E>
		constexpr int CountValid()
		{
			return 0;
		}

		template<typename E, E A, E... B>
		constexpr int CountValid()
		{
			bool is_valid = IsValid<E, A>();
			return CountValid<E, B...>() + (int)is_valid;
		}

		template<typename E, int... I>
		constexpr int InternalElementCount(std::integer_sequence<int, I...> unused)
		{
			return CountValid<E, (E)I...>();
		}

		template<typename E>
		struct ElementCount
		{
			static const int value = InternalElementCount<E>(std::make_integer_sequence<int, 100>());
		};

		template<int Size, typename... Args>
		struct EnumToStringObject
		{ };
#endif

		std::string ShaderStageFlagsToString(ShaderStageFlags flags);

		std::string ColourComponentFlagsToString(ColourComponentFlags flags);

		std::string ImageUsageFlagsToString(ImageUsageFlags flags);

		std::string AccessFlagBitsToString(AccessFlagBits flags);

		std::string PipelineStageFlagsToString(PipelineStageFlags flags);

		std::string ImageAspectFlagsToString(ImageAspectFlags flags);

	}
}