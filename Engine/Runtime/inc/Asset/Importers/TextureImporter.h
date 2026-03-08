#pragma once

#include "Asset/Importers/IAssetImporter.h"

namespace Insight
{
    namespace Runtime
    {
        struct TextureImportContext
        {
            std::vector<Byte> Data;
            int Width;
            int Height;
            int Channels;
        };

        enum class ImageLoader
        {
            Unknown
            , stbi
            , qoi
            , NvidiaTextureTools
        };

        class TextureImporter : public IAssetImporter
        {
        public:
            TextureImporter();

            virtual Ref<Asset> CreateAsset(const AssetInfo* assetInfo) const override;
            virtual void Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const override;
            virtual void ImportAndConvertToEngineFormat(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const override;
            virtual bool CanConvertToEngineFormat() const { return true; }
            virtual Reflect::Type GetAssetType() const override;

            void ImportFromMemory(Asset* asset, const void* data, const u64 dataSize) const;

            void LoadRaw(TextureImportContext& context, const std::string_view fileExtension = std::string_view()) const;

            void CompressToPNG(TextureImportContext& context) const;
            void DecompressFromPNG(TextureImportContext& context) const;

            void CompressToQOI(TextureImportContext& context) const;
            void DecompressFromQOI(TextureImportContext& context) const;

            ImageLoader FileHeaderToImageLoader(const std::vector<u8>& fileData) const;
            ImageLoader FileExtenionToImageLoader(const std::string_view fileExtension) const;

        private:

        };
    }
}