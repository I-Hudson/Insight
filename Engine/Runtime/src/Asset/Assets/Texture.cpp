#include "Asset/Assets/Texture.h"

#include "Graphics/RenderContext.h"
#include "Graphics/PixelFormatExtensions.h"

namespace Insight
{
    namespace Runtime
    {
        Texture::Texture(const AssetInfo* assetInfo)
            : Asset(assetInfo)
        {

        }

        Texture::~Texture()
        {

        }

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
            return m_pixelFormat;
        }

        void Texture::SetTextureData(const void* data, const u64 dataSize)
        {
            if (m_rhiTexture)
            {
                Renderer::FreeTexture(m_rhiTexture);
            }

            const u32 pixelFormatByteSize = PixelFormatExtensions::SizeInBytes(m_pixelFormat);
            const u32 channels = pixelFormatByteSize / 4;
            ASSERT(dataSize == GetWidth() * GetHeight() * GetDepth() * pixelFormatByteSize);
            m_rhiTexture = Renderer::CreateTexture();
            m_rhiTexture->SetName(m_assetInfo->FileName);
            m_rhiTexture->LoadFromData((Byte*)data, GetWidth(), GetHeight(), GetDepth(), channels);
        }

        Graphics::RHI_Texture* Texture::GetRHITexture() const
        {
            return m_rhiTexture;
        }

        void Texture::OnUnload()
        {

        }
    }
}