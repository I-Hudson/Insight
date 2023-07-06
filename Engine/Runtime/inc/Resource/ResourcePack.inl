#include "Resource/ResourcePack.h"
#include "Resource/ResourceManager.h"

#include "FileSystem/FileSystem.h"
#include "Core/Compression.h"

#include <zip.h>
#include <miniz.h>

namespace Insight::Serialisation
{
    void ComplexSerialiser<
        ResroucePack1
        , void
        , Runtime::ResourcePack>::
        operator()(ISerialiser* serialiser
            , Runtime::ResourcePack* resourcePack) const
    {
        ASSERT(serialiser->GetType() == Serialisation::SerialisationTypes::Binary);
        BinarySerialiser* binarySerialiser = static_cast<BinarySerialiser*>(serialiser);

        constexpr const char* c_Resources = "Resources";
        std::unordered_map<Runtime::ResourceId, Runtime::ResourcePack::PackedResource>& resources = resourcePack->m_resources;

        if (serialiser && serialiser->IsReadMode())
        {
            u64 resourceSize = 0;
            serialiser->StartArray(c_Resources, resourceSize);

            for (u32 i = 0; i < resourceSize; ++i)
            {
                Runtime::ResourcePack::PackedResource packedResource;
                packedResource.IsSerialised = true;

                Runtime::ResourceId resourceId;
                resourceId.Deserialise(serialiser);

                Runtime::IResource* resource = Runtime::ResourceManager::GetResource(resourceId);
                ASSERT(resource);

                resource->Deserialise(serialiser);
                packedResource.Resource = resource;

                if (!resource->IsEngineFormat())
                {
                    u64 dataSize = 0;
                    serialiser->Read("DataSize", dataSize);

                    u64 bytePosition = binarySerialiser->GetHeadPosition();

                    std::vector<Byte> data;
                    data.resize(dataSize);
                    //serialiser->Read("Data", data, false);
                    binarySerialiser->Skip(dataSize);

                    packedResource.DataPosition = bytePosition;
                    packedResource.DataSize = dataSize;
                }
                resources[resourceId] = packedResource;
            }

            serialiser->StopArray();
        }
        else
        {
            u64 resourceSize = resources.size();
            serialiser->StartArray(c_Resources, resourceSize);
            
            for (const auto& [filePath, resource] : resources)
            {
                Runtime::ResourceId resourceId = resource.Resource->GetResourceId();
                resourceId.Serialise(serialiser);
                resource.Resource->Serialise(serialiser);
                
                if(!resource.Resource->IsEngineFormat())
                {
                    std::vector<Byte> fileData = FileSystem::ReadFromFile(resource.Resource->GetFilePath(), FileType::Binary);

                    serialiser->Write("DataSize", fileData.size());
                    serialiser->Write("Data", fileData, false);
                }
            }
            
            serialiser->StopArray();
        }
    }
}