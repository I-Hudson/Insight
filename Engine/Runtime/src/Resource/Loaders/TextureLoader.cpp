#include "Resource/Loaders/TextureLoader.h"

#include "FileSystem/FileSystem.h"
#include "Asset/AssetRegistry.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

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

		int width = 0, height = 0, channels = 0, textureSize = 0;
		Byte* textureData = nullptr;
		if (diskFormat == TextureDiskFormat::QOI)
		{
			IS_PROFILE_SCOPE("qoi_decode");
			qoi_desc qoiDesc;
			textureData = static_cast<Byte*>(qoi_decode(fileData.data(), static_cast<int>(fileData.size()), &qoiDesc, 4));
			width = qoiDesc.width;
			height = qoiDesc.height;
			channels = qoiDesc.channels;
		}
		else
		{
			IS_PROFILE_SCOPE("stbi_load_from_memory");
			textureData = stbi_load_from_memory(fileData.data(), static_cast<int>(fileData.size()), &width, &height, &channels, STBI_rgb_alpha);
			channels = STBI_rgb_alpha;
		}

		LoadPixelData loadPixelData;
		if (!textureData)
		{
			return loadPixelData;
		}

		textureSize = width * height * 4;
		loadPixelData.Width = width;
		loadPixelData.Height = height;
		loadPixelData.Depth = 1;
		loadPixelData.Channels = 4;

		loadPixelData.Data.resize(textureSize);
		Platform::MemCopy(loadPixelData.Data.data(), textureData, textureSize);

		if (diskFormat == TextureDiskFormat::QOI)
		{
			QOI_FREE(textureData);
		}
		else
		{
			stbi_image_free(textureData);
		}

		return loadPixelData;
	}
}