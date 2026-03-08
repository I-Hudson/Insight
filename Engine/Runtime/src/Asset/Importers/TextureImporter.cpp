#include "Asset/Importers/TextureImporter.h"
#include "Asset/Assets/Texture.h"
#include "Asset/AssetRegistry.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Platforms/Platform.h"

#include "cmp_compressonatorlib/compressonator.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

#define QOI_IMPLEMENTATION
#ifdef QOI_IMPLEMENTATION
#include "qoi.h"
#endif

#ifdef NVIDIA_Texture_Tools
#include <nvtt/nvtt.h>
#endif

namespace Insight
{
    namespace Runtime
    {
        TextureImporter::TextureImporter()
            : IAssetImporter({ ".png", ".jpeg", ".jpg", ".qoi", ".tga", TextureAsset::GetStaticAssetFileExtension()})
        { }

        Ref<Asset> TextureImporter::CreateAsset(const AssetInfo* assetInfo) const
        {
            return Ref<TextureAsset>(::New<TextureAsset>(assetInfo));
        }

        void TextureImporter::Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
        {
            IS_PROFILE_FUNCTION();

            std::vector<Byte> textureData = AssetRegistry::Instance().LoadAssetData(path);
            ImportFromMemory(asset.Ptr(), textureData.data(), textureData.size());

            Ref<TextureAsset> texture = asset.As<TextureAsset>();
            texture->m_isMemoryAsset = false;
        }

        void TextureImporter::ImportAndConvertToEngineFormat(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
        {
            ASSERT_MSG(FileSystem::GetExtension(assetInfo->FileName) != TextureAsset::GetStaticAssetFileExtension(), "Cannot convert engine texture format.");

            Ref<TextureAsset> textureAsset = asset.As<TextureAsset>();
            textureAsset->m_readableWriteable = true;
            Import(asset, assetInfo, path);
        }

        Reflect::Type TextureImporter::GetAssetType() const
        {
            return TextureAsset::GetStaticTypeInfo().GetType();
        }

        void TextureImporter::ImportFromMemory(Asset* asset, const void* data, const u64 dataSize) const
        {
            std::string_view path = asset->GetAssetInfo()->FilePath;
            if (data == nullptr || dataSize == 0)
            {
                IS_LOG_CORE_ERROR("[TextureImporter::Import] Texture data from path '{}' was empty.", path);
                return;
            }

            TextureImportContext context;
            context.Data.resize(dataSize);
            Platform::MemCopy(context.Data.data(), data, context.Data.size());
            LoadRaw(context);

            if (context.Data.empty())
            {
                IS_LOG_CORE_ERROR("[TextureImporter::Import] Unable to load texture '{}' using, Nvidia texture tools, stbi or QOI.", path.data());
                return;
            }

            CompressToBC3(context);

            TextureAsset* texture = dynamic_cast<TextureAsset*>(asset);
            ASSERT(texture);
            texture->m_width = context.Width;
            texture->m_height = context.Height;
            texture->m_depth = 1;
            texture->m_channels = context.Channels;
            texture->m_pixelFormat = context.PixelFormat;
            texture->m_assetState = AssetState::Loaded;
            texture->m_isMemoryAsset = true;
            texture->SetTextureData(context.Data.data(), context.Data.size());

            if (texture->m_readableWriteable)
            {
                texture->m_textureData.Bytes = std::move(context.Data);
            }
        }

        void TextureImporter::LoadRaw(TextureImportContext& context, const std::string_view fileExtension) const
        {
            void* textureBuffer = nullptr;
            ImageLoader imageLoader = FileExtenionToImageLoader(fileExtension);

            if (imageLoader == ImageLoader::Unknown)
            {
                imageLoader = FileHeaderToImageLoader(context.Data);

                if (imageLoader == ImageLoader::Unknown)
                {
                    FAIL_ASSERT_MSG("[TextureImporter::LoadRaw] Unable to find a vaild image loader.");
                    return;
                }
            }

            if (imageLoader == ImageLoader::stbi)
            {
                imageLoader = ImageLoader::stbi;
                DecompressFromPNG(context);
            }
            else if (fileExtension == ".jpg" || fileExtension == ".jpeg")
            {

            }
            else if (imageLoader == ImageLoader::qoi)
            {
                imageLoader = ImageLoader::qoi;
                DecompressFromQOI(context);
            }
        }

        void TextureImporter::CompressToPNG(TextureImportContext& context) const
        {
        }

        void TextureImporter::DecompressFromPNG(TextureImportContext& context) const
        {
            IS_PROFILE_SCOPE("stbi_load_from_memory");
            void* textureBuffer = stbi_load_from_memory(
                (const stbi_uc*)context.Data.data()
                , context.Data.size()
                , &context.Width
                , &context.Height
                , &context.Channels
                , STBI_rgb_alpha);

            context.Channels = 4;
            const u64 textureSize = context.Width * context.Height * context.Channels;
            context.Data.resize(textureSize);
            Platform::MemCopy(context.Data.data(), textureBuffer, textureSize);

            stbi_image_free(textureBuffer);
        }

        void TextureImporter::CompressToQOI(TextureImportContext& context) const
        {
#ifdef QOI_IMPLEMENTATION
            qoi_desc desc
            {
                context.Width,
                context.Height,
                context.Channels,
                QOI_SRGB
            };

            void* textureBuffer;
            int outLength;
            {
                IS_PROFILE_SCOPE("qoi_encode");
                textureBuffer = qoi_encode(context.Data.data(), &desc, &outLength);
            }

            context.Data.resize(outLength);
            Platform::MemCopy(context.Data.data(), textureBuffer, static_cast<u64>(outLength));
            QOI_FREE(textureBuffer);
#else
            FAIL_ASSERT();
#endif
        }

        void TextureImporter::DecompressFromQOI(TextureImportContext& context) const
        {
            void* textureBuffer;
#ifdef QOI_IMPLEMENTATION
            qoi_desc qoiDesc;
            {
                IS_PROFILE_SCOPE("qoi_decode");
                textureBuffer = qoi_decode(context.Data.data(), context.Data.size(), &qoiDesc, 4);
            }
            context.Width = qoiDesc.width;
            context.Height = qoiDesc.height;
            context.Channels = static_cast<int>(qoiDesc.channels);
            const u64 textureSize = context.Width * context.Height * context.Channels;

            context.Data.resize(textureSize);
            Platform::MemCopy(context.Data.data(), textureBuffer, textureSize);
            QOI_FREE(textureBuffer);
#else
            FAIL_ASSERT();
#endif
        }

        void TextureImporter::CompressToBC3(TextureImportContext& context) const
        {
            IS_PROFILE_FUNCTION();

            // 1. Define the Source Texture (RGBA8)
            CMP_Texture srcTexture = { 0 };
            srcTexture.dwSize = sizeof(CMP_Texture);
            srcTexture.dwWidth = context.Width;
            srcTexture.dwHeight = context.Height;
            srcTexture.dwPitch = srcTexture.dwWidth * context.Channels;
            srcTexture.format = CMP_FORMAT_RGBA_8888;
            srcTexture.dwDataSize = context.Data.size();
            srcTexture.pData = context.Data.data();

            // 2. Define the Destination Texture (BC3)
            CMP_Texture destTexture = { 0 };
            destTexture.dwSize = sizeof(CMP_Texture);
            destTexture.dwWidth = srcTexture.dwWidth;
            destTexture.dwHeight = srcTexture.dwHeight;
            destTexture.dwPitch = 0;
            destTexture.format = CMP_FORMAT_BC3;
            // BC3 uses 1 byte per pixel (16 bytes per 4x4 block)
            destTexture.dwDataSize = CMP_CalculateBufferSize(&destTexture);
            std::vector<Byte> outBuffer(destTexture.dwDataSize);
            destTexture.pData = outBuffer.data();


            // 3. Set Compression Options
            CMP_CompressOptions options = { 0 };
            options.dwSize = sizeof(options);
            options.fquality = 0.5f;            // Quality level: 0.0 (Fast) to 1.0 (High)
            options.nEncodeWith = CMP_GPU_HW;  // Enable OpenCL acceleration for AMD GPUs
            options.bDisableMultiThreading = false;

            // 4. Run Compression
            CMP_ERROR status = CMP_ConvertTexture(&srcTexture, &destTexture, &options, nullptr);

            if (status != CMP_OK) {
                // Log error status
                IS_LOG_CORE_ERROR("[TextureImporter::CompressToBC3] Unable to convert texture data into BC3 format.");
                return;
            }

            context.Data = std::move(outBuffer);
            context.PixelFormat = PixelFormat::BC3_UNorm;
        }

        ImageLoader TextureImporter::FileHeaderToImageLoader(const std::vector<u8>& fileData) const
        {
            if (fileData.size() < 8)
            {
                return ImageLoader::Unknown;
            }

            u8 header[8];
            Platform::MemCopy(header, fileData.data(), 8);

            if (header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF)
            {
                // JPG
                return ImageLoader::stbi;
            }
            else if (header[0] == 0x89 && header[1] == 0x50 && header[2] == 0x4E && header[3] == 0x47 && header[4] == 0x0D && header[5] == 0x0A && header[6] == 0x1A && header[7] == 0x0A)
            {
                // PNG
                return ImageLoader::stbi;
            }
            else if (header[0] == 0x71 && header[1] == 0x6F && header[2] == 0x69 && header[3] == 0x66)
            {
                // QOI
                return ImageLoader::qoi;
            }

            return ImageLoader::Unknown;
        }

        ImageLoader TextureImporter::FileExtenionToImageLoader(const std::string_view fileExtension) const
        {
            if (fileExtension == ".png")
            {
                return ImageLoader::stbi;
            }
            else if (fileExtension == ".jpg" || fileExtension == ".jpeg")
            {

            }
            else if (fileExtension == ".qoi")
            {
                return ImageLoader::qoi;
            }

            return ImageLoader::Unknown;
        }
    }
}
