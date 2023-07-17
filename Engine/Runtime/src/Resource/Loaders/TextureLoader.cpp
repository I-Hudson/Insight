#include "Resource/Loaders/TextureLoader.h"
#include "Resource/Texture2D.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"

#include <stb_image.h>
#include <stb_image_write.h>

namespace Insight::Runtime
{
    TextureLoader::TextureLoader()
        : IResourceLoader({ ".png", ".jpeg", ".jpg", ".qoi" }, { Texture2D::GetStaticResourceTypeId() })
    {
    }

    TextureLoader::~TextureLoader()
    {
    }

    bool TextureLoader::Load(IResource* resource) const
    {
        if (resource == nullptr || !FileSystem::Exists(resource->GetFilePath()))
        {
            IS_CORE_WARN("[TextLoader::Load] resource is null or file path '{}' does not exist.", resource != nullptr ? resource->GetFilePath() : "NULL");
            return false;
        }

		Texture2D* texture = static_cast<Texture2D*>(resource);
		if (texture == nullptr)
		{
			return false;
		}

		std::vector<Byte> fileData = FileSystem::ReadFromFile(texture->GetFilePath(), FileType::Binary);
        resource->LoadFromMemory(fileData.data(), fileData.size());

        return true;
    }

}