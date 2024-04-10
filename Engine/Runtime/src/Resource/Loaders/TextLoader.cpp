#include "Resource/Loaders/TextLoader.h"
#include "Resource/Resource.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"

namespace Insight::Runtime
{
    TextLoader::TextLoader()
        : IResourceLoader({ ".txt"}, { })
    {
    }

    TextLoader::~TextLoader()
    {
    }

    bool TextLoader::Load(IResource* resource) const
    {
        if (resource == nullptr || !FileSystem::Exists(resource->GetFilePath()))
        {
            IS_LOG_CORE_WARN("[TextLoader::Load] resource is null or file path '{}' does not exist.", resource->GetFilePath());
            return false;
        }

        std::vector<Byte> fileData = FileSystem::ReadFromFile(resource->GetFilePath(), FileType::Text);
        //resource->LoadFromMemory(fileData.data(), fileData.size());
        return true;
    }
}