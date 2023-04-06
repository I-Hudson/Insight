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
                data = static_cast<Byte*>(::New(dataSize, Core::MemoryAllocCategory::Resources));

                for (size_t i = 0; i < texture->m_dataSize; ++i)
                {
                    Byte* ptr = data + i;
                    serialiser->Read("", *ptr);
                }
                serialiser->StopArray();
            }
            else
            {
                serialiser->StartArray("DataSize", texture->m_dataSize);
                for (size_t i = 0; i < texture->m_dataSize; ++i)
                {
                    serialiser->Write("", texture->m_dataSize + i);
                }
                serialiser->StopArray();
            }
        }
    }
}