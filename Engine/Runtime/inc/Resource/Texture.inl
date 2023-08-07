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
            constexpr const char* c_TextureData = "TextureData";

            if (serialiser->IsReadMode())
            {
                std::vector<Byte> textureData;
                serialiser->Read(c_TextureData, textureData);

                ASSERT(!data);
                data = static_cast<Byte*>(::NewBytes(textureData.size(), Core::MemoryAllocCategory::Resources));
                texture->m_dataSize = textureData.size();

                Platform::MemCopy(data, textureData.data(), textureData.size());
            }
            else
            {
                ASSERT(texture->m_rawDataPtr && texture->m_dataSize > 0);
                ASSERT(texture->IsEngineFormat() && texture->m_metaData.DiskFormat == Runtime::TextureDiskFormat::QOI);

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
                    serialiser->Write(c_TextureData, compressTextureData);
                }
            }
        }
    }
}