#include "Resource/Loaders/BinaryLoader.h"
#include "Resource/Resource.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"

namespace Insight::Runtime
{
    BinaryLoader::BinaryLoader()
        : IResourceLoader({ }, { })
    {
    }

    BinaryLoader::~BinaryLoader()
    {
    }

    bool BinaryLoader::Load(IResource* resource) const
    {
        if (resource == nullptr || !FileSystem::Exists(resource->GetFilePath()))
        {
            IS_CORE_WARN("[TextLoader::Load] resource is null or file path '{}' does not exist.", resource->GetFilePath());
            return false;
        }

        std::vector<Byte> fileData = FileSystem::ReadFromFile(resource->GetFilePath(), FileType::Binary);
        //resource->LoadFromMemory(fileData.data(), fileData.size());
        return true;
    }
}