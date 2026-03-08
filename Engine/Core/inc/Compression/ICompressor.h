#pragma once

#include "Compression/CompressionTypes.h"

#include <vector>

namespace Insight
{
    namespace Core
    {
        class IS_CORE ICompressor
        {
        public:
            virtual ~ICompressor() { }

            static ICompressor* Create(const CompressionTypes compressionType);
            
            virtual std::vector<Byte> Compress(const void* data, const u64 size) = 0;

            template<typename T>
            std::vector<Byte> Compress(const std::vector<T>& data) { return Compress(data.data(), sizeof(T) * data.size()); }
        };
    }
}