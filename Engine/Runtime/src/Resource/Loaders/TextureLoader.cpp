#include "Resource/Loaders/TextureLoader.h"

#include "FileSystem/FileSystem.h"
#include "Asset/AssetRegistry.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#ifdef NVIDIA_Texture_Tools
#include <nvtt/nvtt.h>
#endif

#include <stb_image.h>
#include <stb_image_write.h>

#ifndef QOI_IMPLEMENTATION
#define QOI_IMPLEMENTATION
#endif
#include <qoi.h>

namespace Insight::Runtime
{
    TextureLoader::TextureLoader()
        : IResourceLoader(std::vector(std::begin(c_TextureLoaderFileExtensions), std::end(c_TextureLoaderFileExtensions)), { Texture2D::GetStaticResourceTypeId() })
    {
    }

    TextureLoader::~TextureLoader()
    {
    }

    bool TextureLoader::Load(IResource* resource) const
    {
		ASSERT(!resource->IsEngineFormat());

        if (resource == nullptr || !AssetRegistry::Instance().GetAsset(resource->GetFilePath()))
        {
            IS_LOG_CORE_WARN("[TextLoader::Load] resource is null or file path '{}' does not exist.", resource != nullptr ? resource->GetFilePath() : "NULL");
            return false;
        }

		Texture2D* texture = static_cast<Texture2D*>(resource);
		if (texture == nullptr)
		{
			return false;
		}

		LoadPixelData pixelsData = LoadPixels(resource->GetFilePath(), texture->m_metaData.DiskFormat);

		if (pixelsData.Data.empty())
		{
			IS_LOG_CORE_ERROR("stbi errro: '%s'.", stbi_failure_reason());
			texture->m_resource_state = EResoruceStates::Failed_To_Load;
			return false;
		}
		
		texture->m_width = pixelsData.Width;
		texture->m_height = pixelsData.Height;
		texture->m_depth = 1;
		texture->m_metaData.PixelFormat = pixelsData.Format;
		texture->UpdateRHITexture(pixelsData.Data.data(), pixelsData.Data.size());
		texture->m_resource_state = EResoruceStates::Loaded;

        return true;
    }

	LoadPixelData TextureLoader::LoadPixels(std::string_view filePath, TextureDiskFormat diskFormat) const
	{
		std::vector<Byte> fileData = AssetRegistry::Instance().LoadAsset(filePath);
		if (fileData.empty())
		{
			fileData = FileSystem::ReadFromFile(filePath, FileType::Binary);
		}

		bool textureIsLoaded = false;
		bool qoiLoad = false;
		bool stbiLoad = false;
		int width = 0, height = 0, channels = 0, textureSize = 0;
		Byte* textureData = nullptr;

#ifdef NVIDIA_Texture_Tools
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
		bool nvttLoadFromMemory = image.loadFromMemory(fileData.data(), static_cast<int>(fileData.size()));

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
		bool nvttCompress = context.compress(image, 0, 0, compressionOptions, outputOptions); // output compressed image

		if (nvttLoadFromMemory && nvttCompress)
		{
			textureIsLoaded = true;
			width = outputHandler.Width;
			height = outputHandler.Height;
			channels = 4;
			textureSize = outputHandler.Size;
			textureData = outputHandler.BufferData.data();
		}
#endif

		if (!textureIsLoaded)
		{
			if (diskFormat == TextureDiskFormat::QOI)
			{
				IS_PROFILE_SCOPE("qoi_decode");
				qoi_desc qoiDesc;
				textureData = static_cast<Byte*>(qoi_decode(fileData.data(), static_cast<int>(fileData.size()), &qoiDesc, 4));
				width = qoiDesc.width;
				height = qoiDesc.height;
				channels = qoiDesc.channels;
				qoiLoad = true;
			}
			else
			{
				IS_PROFILE_SCOPE("stbi_load_from_memory");
				textureData = stbi_load_from_memory(fileData.data(), static_cast<int>(fileData.size()), &width, &height, &channels, STBI_rgb_alpha);
				channels = STBI_rgb_alpha;
				stbiLoad = true;
			}
			textureSize = width * height * 4;
		}

		LoadPixelData loadPixelData;
		if (!textureData)
		{
			return loadPixelData;
		}

		textureSize = textureSize;
		loadPixelData.Width = width;
		loadPixelData.Height = height;
		loadPixelData.Depth = 1;
		loadPixelData.Channels = 4;

		loadPixelData.Data.resize(textureSize);
		Platform::MemCopy(loadPixelData.Data.data(), textureData, textureSize);

		if (qoiLoad)
		{
			loadPixelData.Format = PixelFormat::R8G8B8A8_UNorm;
			QOI_FREE(textureData);
		}
		else if (stbiLoad)
		{
			loadPixelData.Format = PixelFormat::R8G8B8A8_UNorm;
			stbi_image_free(textureData);
		}
		else
		{
			loadPixelData.Format = PixelFormat::BC3_UNorm;
		}

		return loadPixelData;
	}
}