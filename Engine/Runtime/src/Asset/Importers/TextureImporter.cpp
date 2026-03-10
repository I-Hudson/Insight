#include "Asset/Importers/TextureImporter.h"
#include "Asset/Assets/Texture.h"
#include "Asset/AssetRegistry.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Platforms/Platform.h"

#include "Graphics/PixelFormatExtensions.h"

#include "cmp_compressonatorlib/compressonator.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

#define QOI_IMPLEMENTATION
#ifdef QOI_IMPLEMENTATION
#include "qoi.h"
#endif

#ifdef NVIDIA_TEXTURE_TOOLS
#include <nvtt/nvtt.h>
#endif

#include <immintrin.h> // SSE intrinsics

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
            const bool readableWriteable = textureAsset->m_readableWriteable;
            textureAsset->m_readableWriteable = true;
            Import(asset, assetInfo, path);
            textureAsset->m_readableWriteable = readableWriteable;
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
            context.ImageLoader = FileExtenionToImageLoader(fileExtension);

            if (context.ImageLoader == ImageLoader::Unknown)
            {
                context.ImageLoader = FileHeaderToImageLoader(context.Data);

                if (context.ImageLoader == ImageLoader::Unknown)
                {
                    FAIL_ASSERT_MSG("[TextureImporter::LoadRaw] Unable to find a vaild image loader.");
                    return;
                }
            }

            switch (context.ImageLoader)
            {
                case ImageLoader::NVTT:
                case ImageLoader::stbi:
                {
                    DecompressFromPNG(context);
                    break;
                }
                case ImageLoader::qoi:
                {
                    DecompressFromQOI(context);
                    break;
                }

            }
        }

        void TextureImporter::CompressToPNG(TextureImportContext& context) const
        {
        }

        void TextureImporter::DecompressFromPNG(TextureImportContext& context) const
        {
            bool imageLoaded = false;
#if NVIDIA_TEXTURE_TOOLS
            if (context.ImageLoader == ImageLoader::NVTT)
            {
                nvtt::Surface surface;
                bool hasAlpha = true;
                {
                    IS_PROFILE_SCOPE("loadFromMemory");
                    imageLoaded = surface.loadFromMemory(context.Data.data(), context.Data.size(), &hasAlpha);
                }
                if (imageLoaded)
                {
                    context.Width = surface.width();
                    context.Height = surface.height();
                    context.Channels = 4;

                    const float* r = surface.channel(0);
                    const float* g = surface.channel(1);
                    const float* b = surface.channel(2);
                    const float* a = surface.channel(3);
                    // nvtt when compressing to BC format takes the input as BGRA not RGBA.
                    QuantiseTextureData(context, r, g, b, a);
                    context.PixelFormat = PixelFormat::R8G8B8A8_UNorm;
                }
            }
#endif

            if (!imageLoaded)
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
                context.PixelFormat = PixelFormat::R8G8B8A8_UNorm;
            }
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
            context.PixelFormat = PixelFormat::R8G8B8A8_UNorm;

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

#ifdef NVIDIA_TEXTURE_TOOLS
            //if (context.ImageLoader == ImageLoader::NVTT)
            {
                // We need to swap the red and blue channels around. TextureImporter will store raw data as RGBA, NVTT expects BGRA.
                if (context.PixelFormat == PixelFormat::R8G8B8A8_UNorm)
                {
                    SwapRedAndBlueTextureChannels(context);
                    context.PixelFormat == PixelFormat::B8G8R8A8_UNorm;
                }

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

                const auto MessageCallback = [](nvtt::Severity severity, nvtt::Error error, const char* message, const void* userData)
                    {
                        switch (severity)
                        {
                        case nvtt::Severity_Info:
                        {
                            IS_LOG_CORE_INFO("{}", message);
                        }
                        case nvtt::Severity_Warning:
                        {
                            IS_LOG_CORE_WARN("{}", message);
                        }
                        case nvtt::Severity_Error:
                        {
                            IS_LOG_CORE_ERROR("{}", message);
                        }
                        }
                    };

                nvtt::setMessageCallback(MessageCallback, nullptr);

                bool result = false;
                nvtt::useCurrentDevice();
                // First, create an nvtt::Context. Contexts are used both for global settings and for controlling the compression process:
                nvtt::Context nvttContext(true);
                // Now all context compression will be CUDA-accelerated if any system GPU supports it.

                // In NVTT, we use nvtt::Surface to store a single uncompressed image. nvtt::Surface has a method nvtt::Surface::load(), which can be used to load an image file. A typical image loading process looks like this:
                nvtt::Surface image;
                const bool setImage = image.setImage(nvtt::InputFormat_BGRA_8UB, context.Width, context.Height, 1, context.Data.data());
                if (!setImage)
                {
                    IS_LOG_CORE_ERROR("Unable to set NVTT image with uncompessed data");
                    return;
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
                    nvttCompress = nvttContext.compress(image, 0, 0, compressionOptions, outputOptions); // output compressed image
                }

                if (nvttCompress)
                {
                    context.Width = outputHandler.Width;
                    context.Height = outputHandler.Height;
                    context.Channels = 4;
                    const u64 textureSize = outputHandler.Size;
                    context.Data = std::move(outputHandler.BufferData);
                    context.PixelFormat = PixelFormat::BC3_UNorm;

                    return;
                }
                else
                {
                    SwapRedAndBlueTextureChannels(context);
                    context.PixelFormat = PixelFormat::R8G8B8A8_UNorm;
                }
            }
            //else
#endif
            {
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
        }

        void TextureImporter::DecompressFromBC3(TextureImportContext& context) const
        {
            IS_PROFILE_FUNCTION();

            bool textureDecompressed = false;
#ifdef NVIDIA_TEXTURE_TOOLS
            //if (context.ImageLoader == ImageLoader::NVTT)
            {
                const auto MessageCallback = [](nvtt::Severity severity, nvtt::Error error, const char* message, const void* userData)
                    {
                        switch (severity)
                        {
                        case nvtt::Severity_Info:
                        {
                            IS_LOG_CORE_INFO("{}", message);
                        }
                        case nvtt::Severity_Warning:
                        {
                            IS_LOG_CORE_WARN("{}", message);
                        }
                        case nvtt::Severity_Error:
                        {
                            IS_LOG_CORE_ERROR("{}", message);
                        }
                        }
                    };

                nvtt::setMessageCallback(MessageCallback, nullptr);

                bool result = false;
                nvtt::useCurrentDevice();
                // First, create an nvtt::Context. Contexts are used both for global settings and for controlling the compression process:
                nvtt::Context nvttContext(true);
                // Now all context compression will be CUDA-accelerated if any system GPU supports it.

                // In NVTT, we use nvtt::Surface to store a single uncompressed image. nvtt::Surface has a method nvtt::Surface::load(), which can be used to load an image file. A typical image loading process looks like this:
                nvtt::Surface surface;
                textureDecompressed = surface.loadFromMemory(context.Data.data(), context.Data.size());
                if (textureDecompressed)
                {
                    const float* r = surface.channel(0);
                    const float* g = surface.channel(1);
                    const float* b = surface.channel(2);
                    const float* a = surface.channel(3);
                    // nvtt when compressing to BC format takes the input as BGRA not RGBA.
                    QuantiseTextureData(context, r, g, b, a);
                    return;
                }
                else
                {
                    IS_LOG_CORE_ERROR("NVTT Unable to uncompress BC3 image to RGBA.");
                }
            }
            //else
#endif
            {
                // 1. Define the Source Texture (RGBA8)
                CMP_Texture srcTexture = { 0 };
                srcTexture.dwSize = sizeof(CMP_Texture);
                srcTexture.dwWidth = context.Width;
                srcTexture.dwHeight = context.Height;
                srcTexture.dwPitch = srcTexture.dwWidth * context.Channels;
                srcTexture.format = CMP_FORMAT_BC3;
                srcTexture.dwDataSize = context.Data.size();
                srcTexture.pData = context.Data.data();

                // 2. Define the Destination Texture (BC3)
                CMP_Texture destTexture = { 0 };
                destTexture.dwSize = sizeof(CMP_Texture);
                destTexture.dwWidth = srcTexture.dwWidth;
                destTexture.dwHeight = srcTexture.dwHeight;
                destTexture.dwPitch = 0;
                destTexture.format = CMP_FORMAT_RGBA_8888;
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
                context.PixelFormat = PixelFormat::R8G8B8A8_UNorm;
            }
        }

        ImageLoader TextureImporter::FileHeaderToImageLoader(const std::vector<u8>& fileData) const
        {
            if (fileData.size() < 8)
            {
                return ImageLoader::Unknown;
            }

            u8 fileHeader[8];
            Platform::MemCopy(fileHeader, fileData.data(), 8);

            struct FileHeader
            {
                u8 Header[8];
                u8 HeaderSize;
                ImageLoader Loader;
            };

            FileHeader kFileHeaders[] =
            {
                {// JPG
                    { 0xFF, 0xD8, 0xFF },
                    3,
#if NVIDIA_TEXTURE_TOOLS
                    ImageLoader::NVTT,
#else
                    ImageLoader::stbi,
#endif
                },
                {// PNG
                    { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A },
                    8,
#if NVIDIA_TEXTURE_TOOLS
                    ImageLoader::NVTT,
#else
                    ImageLoader::stbi,
#endif
                },
                {// QOI
                    { 0x71, 0x6F, 0x69, 0x66 },
                    4,
                    ImageLoader::qoi
                }
            };

            for (size_t headerIdx = 0; headerIdx < ARRAY_COUNT(kFileHeaders); ++headerIdx)
            {
                const FileHeader& header = kFileHeaders[headerIdx];
                if (Platform::MemCompare(header.Header, fileHeader, header.HeaderSize))
                {
                    return header.Loader;
                }
            }

            return ImageLoader::Unknown;
        }

        ImageLoader TextureImporter::FileExtenionToImageLoader(const std::string_view fileExtension) const
        {
            if (fileExtension == ".png" 
                || fileExtension == ".jpg" 
                || fileExtension == ".jpeg")
            {
#if NVIDIA_TEXTURE_TOOLS
                return ImageLoader::NVTT;
#else
                return ImageLoader::stbi;
#endif
            }
            else if (fileExtension == ".qoi")
            {
                return ImageLoader::qoi;
            }

            return ImageLoader::Unknown;
        }

        // Take texture data from a planner format and transform it into u8 and interleave it.
        void TextureImporter::QuantiseTextureData(TextureImportContext& context, const float* redChannelPtr, const float* greenChannelPtr, const float* blueChannelPtr, const float* alphaChannelPtr) const
        {
            IS_PROFILE_FUNCTION();

            const u64 pixelCount = context.Width * context.Height;
            const u64 textureSize = pixelCount * context.Channels;
            const u64 outputBufferSize = AlignUp(textureSize, 32);
            context.Data.resize(outputBufferSize);

            u8* dst = context.Data.data();
            Platform::MemClear(context.Data.data(), context.Data.size());

            const Core::CPUInformation cpuInfo = Platform::GetCPUInformation();

            if (false && cpuInfo.IsAVX2)
            {
                IS_PROFILE_SCOPE("AVX2");

                const __m256 scale = _mm256_set1_ps(255.0f);
                int i = 0;

                for (; i <= pixelCount - 8; i += 8)
                {
                    __m256 r = _mm256_mul_ps(_mm256_loadu_ps(redChannelPtr + i), scale);
                    __m256 g = _mm256_mul_ps(_mm256_loadu_ps(greenChannelPtr + i), scale);
                    __m256 b = _mm256_mul_ps(_mm256_loadu_ps(blueChannelPtr + i), scale);
                    __m256 a = _mm256_mul_ps(_mm256_loadu_ps(alphaChannelPtr + i), scale);

                    __m256i ri = _mm256_cvtps_epi32(r);
                    __m256i gi = _mm256_cvtps_epi32(g);
                    __m256i bi = _mm256_cvtps_epi32(b);
                    __m256i ai = _mm256_cvtps_epi32(a);

                    // Interleaving 8-bit planes into RGBA in AVX2
                    __m256i rg_16 = _mm256_packus_epi32(ri, gi);
                    __m256i ba_16 = _mm256_packus_epi32(bi, ai);
                    __m256i rgba_8 = _mm256_packus_epi16(rg_16, ba_16);

                    // Fix AVX2 lane shuffling
                    rgba_8 = _mm256_permutevar8x32_epi32(rgba_8, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));

                    _mm256_storeu_si256((__m256i*)(dst + i * 4), rgba_8);
                }

                // Scalar Tail Fallback
                for (; i < pixelCount; ++i)
                {
                    dst[i * 4 + 0] = static_cast<uint8_t>(std::clamp(redChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                    dst[i * 4 + 1] = static_cast<uint8_t>(std::clamp(greenChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                    dst[i * 4 + 2] = static_cast<uint8_t>(std::clamp(blueChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                    dst[i * 4 + 3] = static_cast<uint8_t>(std::clamp(alphaChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                }
            }
            else if (cpuInfo.IsSSE2)
            {
                IS_PROFILE_SCOPE("SSE2");

                const __m128 scale = _mm_set1_ps(255.0f);
                int i = 0;

                for (; i <= pixelCount - 4; i += 4)
                {
                    // 1. Load 4 floats and scale to 0-255
                    __m128 r = _mm_mul_ps(_mm_loadu_ps(redChannelPtr + i), scale);
                    __m128 g = _mm_mul_ps(_mm_loadu_ps(greenChannelPtr + i), scale);
                    __m128 b = _mm_mul_ps(_mm_loadu_ps(blueChannelPtr + i), scale);
                    __m128 a = _mm_mul_ps(_mm_loadu_ps(alphaChannelPtr + i), scale);

                    // 2. Convert to 32-bit int and pack to 8-bit
                    __m128i ri = _mm_cvtps_epi32(r);
                    __m128i gi = _mm_cvtps_epi32(g);
                    __m128i bi = _mm_cvtps_epi32(b);
                    __m128i ai = _mm_cvtps_epi32(a);

                    // 3. Interleave: RRRR/GGGG -> RGRGRGRG -> RGBARGBARGBA
                    __m128i rg = _mm_unpacklo_epi8(_mm_packus_epi16(_mm_packs_epi32(ri, _mm_setzero_si128()), _mm_packus_epi16(_mm_packs_epi32(gi, _mm_setzero_si128()), _mm_setzero_si128())), _mm_setzero_si128());
                    // Simplified Interleave for SSE
                    __m128i r_8 = _mm_packus_epi16(_mm_packs_epi32(ri, _mm_setzero_si128()), _mm_setzero_si128());
                    __m128i g_8 = _mm_packus_epi16(_mm_packs_epi32(gi, _mm_setzero_si128()), _mm_setzero_si128());
                    __m128i b_8 = _mm_packus_epi16(_mm_packs_epi32(bi, _mm_setzero_si128()), _mm_setzero_si128());
                    __m128i a_8 = _mm_packus_epi16(_mm_packs_epi32(ai, _mm_setzero_si128()), _mm_setzero_si128());

                    __m128i rg_lo = _mm_unpacklo_epi8(r_8, g_8);
                    __m128i ba_lo = _mm_unpacklo_epi8(b_8, a_8);
                    __m128i rgba = _mm_unpacklo_epi16(rg_lo, ba_lo);

                    _mm_storeu_si128((__m128i*)(dst + i * 4), rgba);
                }

                // Scalar Tail Fallback
                for (; i < pixelCount; ++i)
                {
                    dst[i * 4 + 0] = static_cast<uint8_t>(std::clamp(redChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                    dst[i * 4 + 1] = static_cast<uint8_t>(std::clamp(greenChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                    dst[i * 4 + 2] = static_cast<uint8_t>(std::clamp(blueChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                    dst[i * 4 + 3] = static_cast<uint8_t>(std::clamp(alphaChannelPtr[i] * 255.0f, 0.0f, 255.0f));
                }
            }
            context.Data.resize(textureSize);
        }

        void TextureImporter::SwapRedAndBlueTextureChannels(TextureImportContext& context) const
        {
            IS_PROFILE_FUNCTION();

            const Core::CPUInformation cpuInfo = Platform::GetCPUInformation();

            if (false && cpuInfo.IsAVX2)
            {

            }
            else if (cpuInfo.IsSSE2)
            {
                const u64 pixelCount = context.Width * context.Height;

                int i = 0;

                for (; i <= pixelCount - 4; i += 4)
                {
                    u8* pixelByteOffset = context.Data.data() + (i * 4);
                    // 1. Load 4 floats and scale to 0-255
                    const __m128i pixelByteData = _mm_loadu_si128((const __m128i*)pixelByteOffset);
                    const __m128i shuffleMash = _mm_setr_epi8(
                        2, 1, 0, 3, 
                        6, 5, 4, 7, 
                        10, 9, 8, 11, 
                        14, 13, 12, 15);
                    const __m128i shuffledPixelBytes = _mm_shuffle_epi8(pixelByteData, shuffleMash);
                    _mm_storeu_si128((__m128i*)pixelByteOffset, shuffledPixelBytes);
                }

                // Scalar Tail Fallback
                for (; i < pixelCount; ++i)
                {
                    u8* pixelByteOffset = context.Data.data() + (i * 4);

                    const u8 red = pixelByteOffset[0];

                    pixelByteOffset[0] = pixelByteOffset[2];
                    pixelByteOffset[2] = red;
                }
            }
        }
    }
}
