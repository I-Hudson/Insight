#include "Resource/ResourcePack.h"
#include "Resource/ResourcePack.inl"

#include "Resource/ResourceManager.h"

#include "Serialisation/Archive.h"

#include "Algorithm/Vector.h"

namespace Insight::Runtime
{
    ResourcePack::ResourcePack(std::string_view path)
        : m_filePath(path)
    {
    }

    ResourcePack::~ResourcePack()
    {
    }

    void ResourcePack::Save()
    {
        IResource::ResourceSerialiserType serialiser(false);
        Serialise(&serialiser);

        Archive archive(m_filePath, ArchiveModes::Write, FileType::Binary);
        archive.Write(serialiser.GetSerialisedData());
        archive.Close();
    }

    std::string_view ResourcePack::GetFilePath() const
    {
        return m_filePath;
    }

    void ResourcePack::LoadAllResources()
    {
        for (const auto& [path, packedResource] : m_resources)
        {
            TObjectPtr<IResource> resrouce = ResourceManager::Load(packedResource.Resource->GetResourceId());
            if (!packedResource.Resource)
            {
                IS_CORE_WARN("[ResourcePack::Load] Something went wrong with '{}'.", resrouce->GetResourceId().GetPath());
            }
        }
    }

    void ResourcePack::UnloadAllResources()
    {
        for (const auto& [path, packedResource] : m_resources)
        {
            ResourceManager::Unload(packedResource.Resource->GetResourceId());
        }
    }

    u64 ResourcePack::GetResourceCount() const
    {
        return m_resources.size();
    }
    
    u64 ResourcePack::GetLoadedResourceCount() const
    {
        u64 count = 0;
        for (const auto& [path, packedResource] : m_resources)
        {
            if (packedResource.Resource->IsLoaded())
            {
                ++count;
            }
        }
        return count;
    }
    
    u64 ResourcePack::GetUnloadedResourceCount() const
    {
        u64 count = 0;
        for (const auto& [path, packedResource] : m_resources)
        {
            if (packedResource.Resource->IsUnloaded() || packedResource.Resource->IsNotLoaded())
            {
                ++count;
            }
        }
        return count;
    }
    
    void ResourcePack::AddResource(IResource* resource)
    { 
        bool contains = m_resources.find(resource->GetResourceId()) != m_resources.end();
        if (!contains)
        {
            m_resources[resource->GetResourceId()] = PackedResource{ resource, 0, 0 };
        }
    }
    
    void ResourcePack::RemoveResource(const IResource* resource)
    {

    }
    
    // -- Begin ISerialisable --
    void ResourcePack::Serialise(Serialisation::ISerialiser* serialiser)
    {
        Serialisation::SerialiserObject<ResourcePack> objectSerialiser;
        objectSerialiser.Serialise(serialiser, *this);
    }
    
    void ResourcePack::Deserialise(Serialisation::ISerialiser* serialiser)
    {
        Serialisation::SerialiserObject<ResourcePack> serialiserObject;
        serialiserObject.Deserialise(serialiser, *this);
    }
    // -- End ISerialisable --
}