#pragma once

#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"

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
                    Runtime::ResourceManager::Create(resouceId);
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

                Serialisation::ISerialiser* resourceSerialiser = Serialisation::ISerialiser::Create(serialiser->GetType(), false);
                Serialisation::ISerialiser* resourceSubserialiser = Serialisation::ISerialiser::Create(serialiser->GetType(), false);
                serialiser->StartArray("Resources", resourcesToSave);
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        // Serialise the ResourceId.
                        const_cast<Runtime::ResourceId&>(pair.first).Serialise(serialiser);

                        if (pair.first.GetTypeId() == Runtime::Texture2D::GetStaticResourceTypeId())
                        {
                            IS_PROFILE_SCOPE("Resource Serialise");

                            resourceSerialiser->Clear();
                            resourceSubserialiser->Clear();

                            {
                                IS_PROFILE_SCOPE("Serialise resource");
                                pair.second->Serialise(resourceSubserialiser);
                            }

                            std::vector<Byte> resourceSerialisedData = resourceSubserialiser->GetSerialisedData();
                            {
                                IS_PROFILE_SCOPE("Write to Serilaise");

                                resourceSerialiser->StartObject("ResourceData");
                                resourceSerialiser->Write("SourceSize", resourceSerialisedData.size());
                                if constexpr (true)
                                {
                                    Core::Compression::Compress(resourceSerialisedData);
                                }
                                resourceSerialiser->Write("CompressedData", resourceSerialisedData);
                                resourceSerialiser->StopObject();
                            }

                            std::string filePath = pair.second->GetFilePath();
                            std::string newExtension = pair.second->GetResourceFileExtension();
                            newExtension += SerialisationTypeToString[(u64)serialiser->GetType()];
                            newExtension += "_Texture_Format_As_Data_Compression";
                            filePath = FileSystem::FileSystem::ReplaceExtension(filePath, newExtension);
                            FileSystem::FileSystem::SaveToFile(resourceSerialisedData, filePath, true);
                        }
                    }
                }
                serialiser->StopArray();
                Delete(resourceSubserialiser);
                Delete(resourceSerialiser);
            }
        }
    }
}