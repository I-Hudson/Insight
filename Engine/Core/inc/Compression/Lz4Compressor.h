#pragma once

#include "Compression/ICompressor.h"

namespace Insight
{
    namespace Core
    {
        class Lz4Compressor : public ICompressor
        {
        public:
            virtual ~Lz4Compressor() override;

            virtual std::vector<Byte> Compress(const void* data, const u64 size) override;
        };
    }
}