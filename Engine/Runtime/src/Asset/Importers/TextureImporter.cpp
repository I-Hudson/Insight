#include "Asset/Importers/TextureImporter.h"
#include "Asset/Assets/Texture.h"
#include "Asset/AssetRegistry.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Platforms/Platform.h"


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

//#define QOI_IMPLEMENTATION
#ifdef QOI_IMPLEMENTATION
#include <qoi.h>
#endif

#ifdef NVIDIA_Texture_Tools
#include <nvtt/nvtt.h>
#endif

namespace Insight
{
    namespace Runtime
    {
        TextureImporter::TextureImporter()
            : IAssetImporter({ ".png", ".jpeg", ".jpg", ".qoi" })
        { }

        Ref<Asset> TextureImporter::CreateAsset(const AssetInfo* assetInfo) const
        {
            return Ref<TextureAsset>(::New<TextureAsset>(assetInfo));
        }

        void TextureImporter::Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
        {
            IS_PROFILE_FUNCTION();

            std::vector<Byte> textureData = AssetRegistry::Instance().LoadAssetData(path);
            ImportFromMemory(asset, textureData.data(), textureData.size());

            Ref<TextureAsset> texture = asset.As<TextureAsset>();
            texture->m_isMemoryAsset = false;
        }

        void TextureImporter::ImportFromMemory(Ref<Asset> asset, const void* data, const u64 dataSize) const
        {
            std::string_view path = asset->GetAssetInfo()->FilePath;
            if (data == nullptr || dataSize == 0)
            {
                IS_LOG_CORE_ERROR("[TextureImporter::Import] Texture data from path '{}' was empty.", path);
                return;
            }

            enum class ImageLoader
            {
                stbi
                , qoi
                , NvidiaTextureTools
            };

            ImageLoader imageLoader = ImageLoader::stbi;
            void* textureBuffer = nullptr;
            u64 textureSize = 0;
            int width, height, channels;
            PixelFormat pixelFormat = PixelFormat::R8G8B8A8_UNorm;

#ifdef NVIDIA_Texture_Tools
            const bool kEnableNVTT = true;

            struct nvttCompressHandler : nvtt::OutputHandler
            {
                virtual ~nvttCompressHandler() override
                {

                }

                /// Indicate the start of a new compressed image that's part of the final texture.
                virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
                {
                    Size = size;
                    Width = width;
                    Height = height;
                    Depth = depth;
                    Face = face;
                    MipLevel = miplevel;
                }

                /// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
                virtual bool writeData(const void* data, int size) override
                {
                    BufferData.resize(size);
                    Platform::MemCopy(BufferData.data(), data, size);
                    return true;
                }

                /// Indicate the end of the compressed image. (New in NVTT 2.1)
                virtual void endImage() override
                {

                }

                int Size;
                int Width;
                int Height;
                int Depth;
                int Face;
                int MipLevel;
                std::vector<u8> BufferData;
            };

            bool result = false;
            nvtt::useCurrentDevice();
            // First, create an nvtt::Context. Contexts are used both for global settings and for controlling the compression process:
            nvtt::Context context;
            context.enableCudaAcceleration(true);
            // Now all context compression will be CUDA-accelerated if any system GPU supports it.

            // In NVTT, we use nvtt::Surface to store a single uncompressed image. nvtt::Surface has a method nvtt::Surface::load(), which can be used to load an image file. A typical image loading process looks like this:
            nvtt::Surface image;
            bool nvttLoadFromMemory = false;
            {
                IS_PROFILE_SCOPE("nvtt - loadFromMemory");
                nvttLoadFromMemory = image.loadFromMemory(data, dataSize);
            }
            // Then, we set up compression options using nvtt::CompressionOptions:
            nvtt::CompressionOptions compressionOptions;
            // Compress to 4-channel, 8-bit-per-pixel BC3:
            compressionOptions.setFormat(nvtt::Format_BC3);

            // See nvtt::Format for all compression formats.
            // Next, we say how to write the compressed data using nvtt::OutputOptions.The simplest case is to assign a filename directly :
            nvtt::OutputOptions outputOptions;
            //outputOptions.setFileName(outputFileName);

            // For more dedicated control of the output stream, you may want to derive a subclass of nvtt::OutputHandler, then use nvtt::OutputOptions::setOutputHandler to redirect the output:
            nvttCompressHandler outputHandler;
            outputOptions.setOutputHandler(&outputHandler);

            // When the above setup is complete, we compress the image using nvtt::Context.
            //context.outputHeader(image, 1, compressionOptions, outputOptions); // output DDS header
            bool nvttCompress = false;
            {
                IS_PROFILE_SCOPE("nvtt - compress");
                nvttCompress = context.compress(image, 0, 0, compressionOptions, outputOptions); // output compressed image
            }

            if (kEnableNVTT && nvttLoadFromMemory && nvttCompress)
            {
                width = outputHandler.Width;
                height = outputHandler.Height;
                channels = 4;
                textureSize = outputHandler.Size;
                textureBuffer = outputHandler.BufferData.data();
                imageLoader = ImageLoader::NvidiaTextureTools;
                pixelFormat = PixelFormat::BC3_UNorm;
            }
#endif

            if (textureBuffer == nullptr)
            {
                IS_PROFILE_SCOPE("stbi_load_from_memory");
                textureBuffer = stbi_load_from_memory((const stbi_uc*)data, dataSize, &width, &height, &channels, STBI_rgb_alpha);
                textureSize = width * height * 4;
                imageLoader = ImageLoader::stbi;
            }
            else if (textureBuffer == nullptr)
            {
#ifdef QOI_IMPLEMENTATION
                imageLoader = ImageLoader::qoi;
                qoi_desc qoiDesc;
                {
                    IS_PROFILE_SCOPE("qoi_decode");
                    textureBuffer = qoi_decode(data, dataSize, &qoiDesc, 4);
                }
                width = qoiDesc.width;
                height = qoiDesc.height;
                channels = static_cast<int>(qoiDesc.channels);
                textureSize = width * height * 4;
                imageLoader = ImageLoader::qoi;
#endif
            }

#if 0
            {
                IS_PROFILE_SCOPE("qoi_encode");
                qoi_desc qoiDesc = {};
                qoiDesc.width = width;
                qoiDesc.height = height;
                qoiDesc.channels = 4;
                qoiDesc.colorspace = QOI_SRGB;

                int outLength = -1;
                void* qoiEncode = qoi_encode(textureBuffer, &qoiDesc, &outLength);
                FileSystem::SaveToFile((Byte*)qoiEncode, outLength, std::string(path) + ".qoi", FileType::Binary, true);
            }
#endif

            if (textureBuffer == nullptr)
            {
                IS_LOG_CORE_ERROR("[TextureImporter::Import] Unable to load texture '{}' using, Nvidia texture tools, stbi or QOI.", path.data());
                return;
            }

            Ref<TextureAsset> texture = asset.As<TextureAsset>();
            texture->m_width = width;
            texture->m_height = height;
            texture->m_depth = 1;
            texture->m_channels = 4;
            texture->m_pixelFormat = pixelFormat;
            texture->m_assetState = AssetState::Loaded;
            texture->m_isMemoryAsset = true;
            texture->SetTextureData(textureBuffer, textureSize);

            switch (imageLoader)
            {
            case ImageLoader::stbi:
            {
                stbi_image_free(textureBuffer);
                break;
            }
            case ImageLoader::qoi:
            {
                free(textureBuffer);
                break;
            }
            }
        }
    }
}
