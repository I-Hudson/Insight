#pragma once

#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourcePack.h"

#include "Serialisation/Archive.h"

#include "FileSystem/FileSystem.h"
#include "Resource/Texture2D.h"

#include "Core/Compression.h"
#include "Core/Profiler.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<ResourceDatabase1, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            if (serialiser->IsReadMode())
            {
                u64 resourcesToSave = 0;
                serialiser->StartArray("Resources", resourcesToSave);
                for (u64 i = 0; i < resourcesToSave; ++i)
                {
                    Runtime::ResourceId resouceId;
                    resouceId.Deserialise(serialiser);

                    TObjectPtr<Runtime::IResource> resource = Runtime::ResourceManager::Create(resouceId);
                    if (resource)
                    {
                        resource->Deserialise(serialiser);
                    }
                }
                serialiser->StopArray();
            }
            else
            {
                u64 resourcesToSave = 0;
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        ++resourcesToSave;
                    }
                }

                serialiser->StartArray("Resources", resourcesToSave);
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        const_cast<Runtime::ResourceId&>(pair.first).Serialise(serialiser);
                        pair.second->Serialise(serialiser);
                    }
                }
                serialiser->StopArray();
            }
        }

        void ComplexSerialiser<ResourceDatabase2, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            IS_PROFILE_FUNCTION();

            if (serialiser->IsReadMode())
            {
                u64 resourcesToSave = 0;
                serialiser->StartArray("Resources", resourcesToSave);
                for (u64 i = 0; i < resourcesToSave; ++i)
                {
                    Runtime::ResourceId resouceId;
                    resouceId.Deserialise(serialiser);
                    Runtime::IResource* resource = Runtime::ResourceManager::Create(resouceId).Get();

                    // Resource is not an engine format. Load additional data which has been saved 
                    // to fill in engine fields like GUIDs.
                    if (!resource->IsEngineFormat())
                    {
                        resource->Deserialise(serialiser);
                    }
                }
                serialiser->StopArray();
            }
            else
            {
                u64 resourcesToSave = 0;
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        ++resourcesToSave;
                    }
                }

                serialiser->StartArray("Resources", resourcesToSave);
                for (auto const& [resourceId, resource] : map)
                {
                    if (!resource->IsDependentOnAnotherResource())
                    {
                        // Serialise the ResourceId.
                        const_cast<Runtime::ResourceId&>(resourceId).Serialise(serialiser);

                        // If the resource is not an engine resource. Then we save additional
                        // data for help when loading it. An example is GUIDs which have been assigned.
                        if (!resource->IsEngineFormat())
                        {
                            resource->Serialise(serialiser);
                        }
                    }
                }
                serialiser->StopArray();
            }
        }

        void ComplexSerialiser<ResourceDatabase3, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            IS_PROFILE_FUNCTION();

            constexpr const char* c_Resources = "Resources";
            constexpr const char* c_Guid = "Guid";

            if (serialiser->IsReadMode())
            {
                Serialisation::PropertyDeserialiser<Core::GUID> guidDeserialiser;
                u64 resourcesToSave = 0;
                serialiser->StartArray(c_Resources, resourcesToSave);
                for (u64 i = 0; i < resourcesToSave; ++i)
                {
                    Runtime::ResourceId resouceId;
                    resouceId.Deserialise(serialiser);

                    bool resourceExistsOnDisk = FileSystem::Exists(resouceId.GetPath());
                    if (resourceExistsOnDisk)
                    {
                        resourceDatabase->AddResource(resouceId, true);
                    }

                    std::string guidString;
                    serialiser->Read(c_Guid, guidString);
                    Core::GUID guid = guidDeserialiser(guidString);

                    if (!resourceExistsOnDisk)
                    {
                        ASSERT(resourceDatabase->m_missingResources.find(resouceId) == resourceDatabase->m_missingResources.end());
                        resourceDatabase->m_missingResources[resouceId] = guid;
                    }
                }
                serialiser->StopArray();
            }
            else
            {
                u64 resourcesToSave = 0;
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        ++resourcesToSave;
                    }
                }

                Serialisation::PropertySerialiser<Core::GUID> guidSerialiser;
                serialiser->StartArray(c_Resources, resourcesToSave);
                for (auto const& [resourceId, resource] : map)
                {
                    if (!resource->IsDependentOnAnotherResource())
                    {
                        // Serialise the ResourceId.
                        const_cast<Runtime::ResourceId&>(resourceId).Serialise(serialiser);
                        serialiser->Write(c_Guid, guidSerialiser(resource->GetGuid()));
                    }
                }
                serialiser->StopArray();
            }
        }

        void ComplexSerialiser<ResourceDatabase4, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            IS_PROFILE_FUNCTION();

            constexpr const char* c_Resources = "Resources";

            if (serialiser->IsReadMode())
            {
                Serialisation::SerialiserObject<Runtime::IResource> IResourceDeserialiser;

                u64 resourcesToSave = 0;
                serialiser->StartArray(c_Resources, resourcesToSave);
                for (u64 i = 0; i < resourcesToSave; ++i)
                {
                    Runtime::ResourceId resouceId;
                    resouceId.Deserialise(serialiser);

                    Runtime::IResource* createdResource = resourceDatabase->AddResource(resouceId).Get();
                    if (createdResource == nullptr)
                    {
                        Runtime::IResource dummy("dummy");
                        IResourceDeserialiser.Deserialise(serialiser, dummy);
                        continue;
                    }

                    IResourceDeserialiser.Deserialise(serialiser, *createdResource);
                }
                serialiser->StopArray();
            }
            else
            {
                u64 resourcesToSave = 0;
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        ++resourcesToSave;
                    }
                }

                Serialisation::SerialiserObject<Runtime::IResource> IResourceSerialiser;

                serialiser->StartArray(c_Resources, resourcesToSave);
                for (auto const& [resourceId, resource] : map)
                {
                    if (!resource->IsDependentOnAnotherResource())
                    {
                        // Serialise the ResourceId.
                        const_cast<Runtime::ResourceId&>(resourceId).Serialise(serialiser);
                        IResourceSerialiser.Serialise(serialiser, *resource.Get());
                    }
                }
                serialiser->StopArray();
            }
        }


        void ComplexSerialiser<ResourceDatabasePacks1, std::vector<Runtime::ResourcePack*>, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, std::vector<Runtime::ResourcePack*>& packs, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            IS_PROFILE_FUNCTION();

            constexpr const char* c_ResourcePacks = "ResourcePacks";
            constexpr const char* c_PackFilePath = "PackFilePath";
            constexpr const char* c_Pack = "Pack";
            constexpr const char* c_ResourceGuids = "ResourceGuids";
            constexpr const char* c_Guid = "Guid";

            if (serialiser->IsReadMode())
            {
                u64 packsSize = 0;
                serialiser->StartArray(c_ResourcePacks, packsSize);

                for (size_t packIdx = 0; packIdx < packsSize; ++packIdx)
                {
                    serialiser->StartObject(c_Pack);
                    std::string packFilePath;
                    serialiser->Read(c_PackFilePath, packFilePath);


                    Runtime::ResourcePack* pack = nullptr;
                    if (FileSystem::Exists(packFilePath))
                    {
                        //If we don't have a resource pack at the loction know then don't create one.
                        pack = Runtime::ResourceManager::CreateResourcePack(packFilePath);
                    }

                    u64 resourceGuidsSize = 0;
                    serialiser->StartArray(c_ResourceGuids, resourceGuidsSize);
                    for (size_t resourceGuidIdx = 0; resourceGuidIdx < resourceGuidsSize; ++resourceGuidIdx)
                    {
                        std::string resourceGuidStr;
                        serialiser->Read(c_Guid, resourceGuidStr);

                        Core::GUID resourceGuid;
                        resourceGuid.StringToGuid(resourceGuidStr);

                        if (pack)
                        {
                            Runtime::IResource* resourceToAdd = Runtime::ResourceManager::GetResourceFromGuid(resourceGuid);
                            if (resourceToAdd)
                            {
                                pack->AddResource(resourceToAdd);
                            }
                            else
                            {
                                IS_CORE_WARN("[ComplexSerialiser::ResourceDatabasePacks1] Resource guid '{}' is null. Resource has not been added to pack '{}'."
                                    , resourceGuidStr, pack->GetFilePath().data());
                            }
                        }
                    }
                    serialiser->StopArray();

                    serialiser->StopObject();
                }

                serialiser->StopArray();
            }
            else
            {
                u64 packsSize = packs.size();
                serialiser->StartArray(c_ResourcePacks, packsSize);

                for (Runtime::ResourcePack*& pack : packs)
                {
                    serialiser->StartObject(c_Pack);
                    serialiser->Write(c_PackFilePath, pack->GetFilePath().data());

                    const std::unordered_map<Runtime::ResourceId, Runtime::ResourcePack::PackedResource>& packResources 
                        = pack->m_resources;

                    u64 resourceGuidsSize = packResources.size();
                    serialiser->StartArray(c_ResourceGuids, resourceGuidsSize);
                    for (const auto& [resourceId, packedResource] : packResources)
                    {
                        serialiser->Write(c_Guid, packedResource.Resource->GetGuid().ToString());
                    }
                    serialiser->StopArray();

                    serialiser->StopObject();
                }

                serialiser->StopArray();
            }
        }
    }
}