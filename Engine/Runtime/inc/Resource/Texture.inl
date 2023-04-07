#pragma once

#include "Resource/Texture.h"

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
                
                std::vector<Byte> compressTextureData = texture->PNG();
                u64 dataSize = compressTextureData.size(); //texture->m_dataSize;
                serialiser->StartArray("DataSize", dataSize);
                for (size_t i = 0; i < dataSize; ++i)
                {
                    Byte* data = compressTextureData.data() + i;//texture->m_rawDataPtr + i;
                    serialiser->Write("", *data);
                }
                serialiser->StopArray();
            }
        }
    }
}