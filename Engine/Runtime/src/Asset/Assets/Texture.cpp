#include "Asset/Assets/Texture.h"

#include "Graphics/RenderContext.h"
#include "Graphics/PixelFormatExtensions.h"

namespace Insight
{
    namespace Runtime
    {
        TextureAsset::TextureAsset(const AssetInfo* assetInfo)
            : Asset(assetInfo)
        {

        }

        TextureAsset::~TextureAsset()
        {

        }

        u32 TextureAsset::GetWidth() const
        {
            return m_width;
        }

        u32 TextureAsset::GetHeight() const
        {
            return m_height;
        }
        
        u32 TextureAsset::GetDepth() const
        {
            return m_depth;
        }
        
        PixelFormat TextureAsset::GetFormat() const
        {
            return m_pixelFormat;
        }

        void TextureAsset::SetTextureData(const void* data, const u64 dataSize)
        {
            if (m_rhiTexture)
            {
                Renderer::FreeTexture(m_rhiTexture);
            }

            const u32 pixelFormatByteSize = PixelFormatExtensions::SizeInBytes(m_pixelFormat);
            ASSERT(dataSize == GetWidth() * GetHeight() * GetDepth() * m_channels);
            m_rhiTexture = Renderer::CreateTexture();
            m_rhiTexture->SetName(m_assetInfo->FileName);
            m_rhiTexture->LoadFromData((Byte*)data, GetWidth(), GetHeight(), GetDepth(), m_channels);
        }

        Graphics::RHI_Texture* TextureAsset::GetRHITexture() const
        {
            return m_rhiTexture;
        }

        void TextureAsset::OnUnload()
        {

        }
    }
}