#include "Compression/Lz4Compressor.h"
#include "Core/Profiler.h"
#include "Core/Logger.h"

#include "lz4.h"

namespace Insight
{
    namespace Core
    {
        Lz4Compressor::~Lz4Compressor()
        {
        }

        std::vector<Byte> Lz4Compressor::Compress(const void* data, const u64 size)
        {
            IS_PROFILE_FUNCTION();

            const int src_size = static_cast<int>(size);

            // LZ4 provides a function that will tell you the maximum size of compressed output based on input data via LZ4_compressBound().
            const int max_dst_size = LZ4_compressBound(src_size);
            // We will use that size for our destination boundary when allocating space.
            std::vector<Byte> compressedData;
            compressedData.resize(static_cast<u64>(max_dst_size));

            // That's all the information and preparation LZ4 needs to compress *src into* compressed_data.
            // Invoke LZ4_compress_default now with our size values and pointers to our memory locations.
            // Save the return value for error checking.
            const int compressed_data_size = LZ4_compress_default(static_cast<const char*>(data), (char*)compressedData.data(), src_size, max_dst_size);

            // Check return_value to determine what happened.
            if (compressed_data_size <= 0)
            {
                IS_LOG_CORE_ERROR("A 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data.");
                return {};
            }
            if (compressed_data_size > 0)
            {
                IS_LOG_CORE_INFO("[Compression::Compress] We successfully compressed some data! Ratio: {0:.2f}",
                    (float)compressed_data_size / src_size);
            }

            // Not only does a positive return_value mean success, the value returned == the number of bytes required.
            // You can use this to realloc() *compress_data to free up memory, if desired.  We'll do so just to demonstrate the concept.
            compressedData.resize(static_cast<u64>(compressed_data_size));

            return compressedData;
        }
    }
}