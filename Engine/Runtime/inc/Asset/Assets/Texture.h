#pragma once

#include "Asset/Asset.h"
#include "Runtime/Defines.h"

#include "Graphics/PixelFormat.h"

#include "Generated/Texture_reflect_generated.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Texture;
    }

    namespace Runtime
    {
        class TextureImporter;
        class ModelAsset;

        enum class TextureAssetTypes : u8
        {
            Diffuse,
            Normal,
            Specular,

            Count
        };
        constexpr const char* TextureAssetTypesToString[]
        {
            "Diffuse",
            "Normal",
            "Specular"
        };
        static_assert(ARRAY_COUNT(TextureAssetTypesToString) == static_cast<u64>(TextureAssetTypes::Count));

        REFLECT_CLASS();
        class IS_RUNTIME TextureAsset : public Asset
        {
            REFLECT_GENERATED_BODY();
        public:
            TextureAsset(const AssetInfo* assetInfo);
            virtual ~TextureAsset() override;

            IS_OBJECT(TextureAsset);
            IS_ASSERT(TextureAsset);
            IS_SERIALISABLE_H(TextureAsset);

            u32 GetWidth() const;
            u32 GetHeight() const;
            u32 GetDepth() const;
            PixelFormat GetFormat() const;

            void SetReadableWriteable(const bool readableWriteable) { m_readableWriteable = readableWriteable; }
            bool HasTextureData() const { return m_textureData.Bytes.empty(); }

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
            bool m_readableWriteable = false;
            
            struct TextureData
            {
                std::vector<Byte> Bytes;
            };
            TextureData m_textureData; // Locally stored texture data. Not always present.
            Graphics::RHI_Texture* m_rhiTexture = nullptr;

            friend class TextureImporter;
            friend class ModelAsset;
        };
    }

    namespace Serialisation
    {
        struct TextureAssetTextureData {};
        struct TextureAssetTextureDataQOI {};
        struct TextureAssetTextureDataLZ4 {};
        struct TextureAssetTextureDataBC3{};
    }

    OBJECT_SERIALISER(Runtime::TextureAsset, 4,
        SERIALISE_BASE(Runtime::Asset, 1, 0)
        SERIALISE_PROPERTY(u32, m_width, 1, 0)
        SERIALISE_PROPERTY(u32, m_height, 1, 0)
        SERIALISE_PROPERTY(u32, m_depth, 1, 0)
        SERIALISE_PROPERTY(u32, m_channels, 1, 0)
        SERIALISE_PROPERTY(PixelFormat, m_pixelFormat, 1, 0)
        SERIALISE_COMPLEX(Serialisation::TextureAssetTextureData, m_textureData, 1, 2)
        SERIALISE_COMPLEX(Serialisation::TextureAssetTextureDataQOI, m_textureData, 2, 3)
        SERIALISE_COMPLEX(Serialisation::TextureAssetTextureDataLZ4, m_textureData, 3, 4)
        SERIALISE_COMPLEX(Serialisation::TextureAssetTextureDataBC3, m_textureData, 4, 0)
    );

    OBJECT_SERIALISER_META(Runtime::TextureAsset, 1,
        SERIALISE_ADD_PROPERTY(u32, m_width, 1)
    );
}

#include "Texture.inl"