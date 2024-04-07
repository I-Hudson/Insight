#pragma once

#include "Asset/Asset.h"

#include "Graphics/PixelFormat.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Texture;
    }

    class TextureImporter;

    namespace Runtime
    {
        class TextureAsset : public Asset
        {
        public:
            TextureAsset(const AssetInfo* assetInfo);
            virtual ~TextureAsset() override;

            u32 GetWidth() const;
            u32 GetHeight() const;
            u32 GetDepth() const;
            PixelFormat GetFormat() const;

            void SetTextureData(const void* data, const u64 dataSize);
            Graphics::RHI_Texture* GetRHITexture() const;

            // BEGIN Asset
        protected:
            virtual void OnUnload() override;
            // END Asset

        private:
            u32 m_width = 0;
            u32 m_height = 0;
            u32 m_depth = 0;
            u32 m_channels = 0;
            PixelFormat m_pixelFormat = PixelFormat::Unknown;
            
            Graphics::RHI_Texture* m_rhiTexture = nullptr;

            friend class TextureImporter;
        };
    }
}