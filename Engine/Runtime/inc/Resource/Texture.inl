#pragma once

#include "Resource/Texture.h"

#include "Core/Compression.h"
#include "Core/Profiler.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<SerialiseTextureData, Byte*, Runtime::Texture>::operator()
            (ISerialiser* serialiser, Byte*& data, Runtime::Texture* texture) const
        {
            ASSERT(serialiser);
            if (serialiser->IsReadMode())
            {
                u64 dataSize = 0;
                serialiser->StartArray("DataSize", dataSize);

                ASSERT(!data);
                data = static_cast<Byte*>(::NewBytes(dataSize, Core::MemoryAllocCategory::Resources));

                for (size_t i = 0; i < texture->m_dataSize; ++i)
                {
                    Byte* ptr = data + i;
                    serialiser->Read("", *ptr);
                }
                serialiser->StopArray();
            }
            else
            {             
                std::vector<Byte> compressTextureData;
                u64 dataSize = texture->m_dataSize;

                compressTextureData.resize(dataSize);
                Platform::MemCopy(compressTextureData.data(), texture->m_rawDataPtr, dataSize);
                
                if constexpr (true)
                {
                    Core::Compression::Compress(compressTextureData);
                }

                {
                    IS_PROFILE_SCOPE("Write texture data");
                    serialiser->Write("TextureData", compressTextureData);
                }
            }
        }
    }
}