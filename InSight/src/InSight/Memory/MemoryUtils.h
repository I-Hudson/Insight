#pragma once

#include "Insight/Core.h"
#include "Insight/InsightAlias.h"

namespace Insight
{
	namespace Memory
	{
		class IS_API MemoryUtlis
		{
		public:
			static const Byte Alignment = 8;

			static const Size CalculatePadding(const Size baseAddress, const Byte alignment) {
				const Size multiplier = (baseAddress / alignment) + 1;
				const Size alignedAddress = multiplier * alignment;
				const Size padding = alignedAddress - baseAddress;
				return padding;
			};

			static const Size CalculatePaddingWithHeader(const Size baseAddress, const Byte alignment, const Size headerSize) {
				Size padding = CalculatePadding(baseAddress, alignment);
				Size neededSpace = headerSize;

				if (padding < neededSpace)
				{
					// Header does not fit - Calculate next aligned address that header fits
					neededSpace -= padding;

					// How many alignments I need to fit the header        
					if (neededSpace % alignment > 0)
					{
						padding += alignment * (1 + (neededSpace / alignment));
					}
					else
					{
						padding += alignment * (neededSpace / alignment);
					}
				}

				return padding;
			}
		};
	}
}
