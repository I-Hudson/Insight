#include "Compression/ICompressor.h"
#include "Core/Memory.h"

#include "Compression/Lz4Compressor.h"

namespace Insight
{
    namespace Core
    {
        ICompressor* ICompressor::Create(const CompressionTypes compressionType)
        {
            switch (compressionType)
            {
                case CompressionTypes::lz4:
                {
                    return ::New<Lz4Compressor>();
                }
            }
            return nullptr;
        }
    }
}