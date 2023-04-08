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
                std::vector<Byte> textureData;
                serialiser->Read("DataSize", textureData);

                ASSERT(!data);
                data = static_cast<Byte*>(::NewBytes(textureData.size(), Core::MemoryAllocCategory::Resources));
                texture->m_dataSize = textureData.size();

                Platform::MemCopy(data, textureData.data(), textureData.size());
            }
            else
            {             
                std::vector<Byte> compressTextureData;
                u64 dataSize = texture->m_dataSize;

                compressTextureData.resize(dataSize);
                Platform::MemCopy(compressTextureData.data(), texture->m_rawDataPtr, dataSize);
                
                if constexpr (false)
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