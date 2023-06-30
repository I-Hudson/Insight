#include "Resource/ResourcePack.h"
#include "Resource/ResourcePack.inl"

#include "Resource/ResourceManager.h"

#include "Serialisation/Archive.h"

#include "Algorithm/Vector.h"

namespace Insight::Runtime
{
    ResourcePack::ResourcePack(std::string_view path)
        : IResource(std::move(path))
    {
    }

    ResourcePack::~ResourcePack()
    {
    }

    void ResourcePack::Save()
    {
        IResource::ResourceSerialiserType serialiser(false);
        Serialise(&serialiser);

        Archive archive(GetFilePath(), ArchiveModes::Write, FileType::Binary);
        archive.Write(serialiser.GetSerialisedData());
        archive.Close();
    }

    void ResourcePack::LoadAllResources()
    {
        for (const IResource* resource : m_resources)
        {
            TObjectPtr<IResource> resrouce = ResourceManager::Load(resource->GetResourceId());
            if (!resource)
            {
                IS_CORE_WARN("[ResourcePack::Load] Something went wrong with '{}'.", resrouce->GetResourceId().GetPath());
            }
        }
    }

    void ResourcePack::UnloadAllResources()
    {
        for (const IResource* resource : m_resources)
        {
            ResourceManager::Unload(resource->GetResourceId());
        }
    }

    u64 ResourcePack::GetResourceCount() const
    {
        return m_resources.size();
    }
    
    u64 ResourcePack::GetLoadedResourceCount() const
    {
        u64 count = 0;
        for (const IResource* resource : m_resources)
        {
            if (resource->IsLoaded()) 
            {
                ++count;
            }
        }
        return count;
    }
    
    u64 ResourcePack::GetUnloadedResourceCount() const
    {
        u64 count = 0;
        for (const IResource* resource : m_resources)
        {
            if (resource->IsUnloaded() || resource->IsNotLoaded())
            {
                ++count;
            }
        }
        return count;
    }
    
    void ResourcePack::AddResource(IResource* resource)
    { 
        bool contains = Algorithm::VectorContainsIf(m_resources, [&resource](const IResource* res)
            {
                return resource->GetResourceId() == res->GetResourceId();
            });
        if (!contains)
        {
            m_resources.push_back(resource);
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

    // -- Begin IResource --
    void ResourcePack::Load()
    {
        if (m_resource_state == EResoruceStates::Loaded)
        {
            return;
        }

        Serialisation::JsonSerialiser serialiser;
        Deserialise(&serialiser);

        m_resource_state = EResoruceStates::Loaded;
    }
    
    void ResourcePack::LoadFromMemory(const void* data, u64 size_in_bytes)
    {
    }
    
    void ResourcePack::UnLoad()
    {
        UnloadAllResources();
        m_resource_state = EResoruceStates::Unloaded;
    }
    // -- End IResource --
}