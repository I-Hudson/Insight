#include "Resource/Texture.h"
#include "Resource/Texture.inl"

#include "Graphics/RHI/RHI_Texture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Insight
{
    namespace Runtime
    {
        IS_SERIALISABLE_CPP(Texture)

            u32 Texture::GetWidth() const
        {
            return m_width;
        }

        u32 Texture::GetHeight() const
        {
            return m_height;
        }

        u32 Texture::GetDepth() const
        {
            return m_depth;
        }

        PixelFormat Texture::GetFormat() const
        {
            return m_format;
        }

        Graphics::RHI_Texture* Texture::GetRHITexture() const
        {
            return m_rhi_texture;
        }

        std::vector<Byte> Texture::PNG()
        {
            struct Context
            {
                std::vector<Byte> compressTextureData;
                u64 Size = 0;
            };
            Context textureContext;

            stbi_write_jpg_to_func([](void* context, void* data, int size)
            {
                Context* textureData = reinterpret_cast<Context*>(context);
                textureData->compressTextureData.resize(textureData->Size + size);

                void* dst = textureData->compressTextureData.data() + textureData->Size;
                Platform::MemCopy(dst, data, size);
                textureData->Size += size;
            },
            & textureContext, GetWidth(), GetHeight(), 4, m_rawDataPtr, 0);
            return textureContext.compressTextureData;
        }
    }
}