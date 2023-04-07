#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <vector>

namespace Insight
{
    namespace Core
    {
        /// @brief Utility class to be used for compressing an decompressing data.
        class IS_CORE Compression
        {
        public:
            static bool Compress(std::vector<Byte>& sourceData);
            static void Decompress(std::vector<Byte>& compressedData, u64 sourceSize);
        };
    }
}