#pragma once

#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"

#include "FileSystem/FileSystem.h"
#include "Resource/Texture2D.h"

#include <lz4.h>

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

                Serialisation::ISerialiser* resourceSerialiser = Serialisation::ISerialiser::Create(SerialisationTypes::Json, false);
                serialiser->StartArray("Resources", resourcesToSave);
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        // Serialise the ResourceId.
                        const_cast<Runtime::ResourceId&>(pair.first).Serialise(serialiser);

                        if (pair.first.GetTypeId() == Runtime::Texture2D::GetStaticResourceTypeId())
                        {
                            resourceSerialiser->Clear();
                            pair.second->Serialise(resourceSerialiser);
                            std::vector<Byte> resourceSerialisedData = resourceSerialiser->GetSerialisedData();
                            std::vector<Byte> compressedData;

                            if constexpr (false)
                            {
                                // TODO: ALL OF THIS SHOULD BE IN AN COMPRESS CLASS WHICH HANDLES COMPRESSING AND UNCOMPRESSING.
                                const int src_size = static_cast<int>(resourceSerialisedData.size());

                                // LZ4 provides a function that will tell you the maximum size of compressed output based on input data via LZ4_compressBound().
                                const int max_dst_size = LZ4_compressBound(src_size);
                                // We will use that size for our destination boundary when allocating space.
                                compressedData.resize(static_cast<u64>(max_dst_size));

                                // That's all the information and preparation LZ4 needs to compress *src into* compressed_data.
                                // Invoke LZ4_compress_default now with our size values and pointers to our memory locations.
                                // Save the return value for error checking.
                                const int compressed_data_size = LZ4_compress_default((char*)resourceSerialisedData.data(), (char*)compressedData.data(), src_size, max_dst_size);

                                // Check return_value to determine what happened.
                                if (compressed_data_size <= 0)
                                {
                                    FAIL_ASSERT_MSG("A 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data.");
                                }
                                if (compressed_data_size > 0)
                                {
                                    IS_CORE_INFO("[ComplexSerialiser<ResourceDatabase2>] We successfully compressed some data! Ratio: {0:.2f}",
                                        (float)compressed_data_size / src_size);
                                }

                                // Not only does a positive return_value mean success, the value returned == the number of bytes required.
                                // You can use this to realloc() *compress_data to free up memory, if desired.  We'll do so just to demonstrate the concept.
                                compressedData.resize(static_cast<u64>(compressed_data_size));
                            }

                            std::string filePath = pair.second->GetFilePath();
                            filePath = FileSystem::FileSystem::ReplaceExtension(filePath, pair.second->GetResourceFileExtension());
                            FileSystem::FileSystem::SaveToFile(compressedData, filePath);
                        }
                    }
                }
                serialiser->StopArray();
                Delete(resourceSerialiser);
            }
        }
    }
}