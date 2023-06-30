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

        constexpr const char* c_Resources = "Resources";
        std::vector<Runtime::IResource*>& resources = resourcePack->m_resources;

        if (serialiser && serialiser->IsReadMode())
        {
            u64 resourceSize = 0;
            serialiser->StartArray(c_Resources, resourceSize);

            for (u32 i = 0; i < resourceSize; ++i)
            {
                Runtime::ResourceId resourceId;
                resourceId.Deserialise(serialiser);

                Runtime::IResource* resource = Runtime::ResourceRegister::CreateResource(resourceId.GetTypeId(), resourceId.GetPath());
                resource->Deserialise(serialiser);

                if (!resource->IsEngineFormat())
                {
                    u64 dataSize = 0;
                    serialiser->Read("DataSize", dataSize);

                    std::vector<Byte> data;
                    data.resize(dataSize);
                    serialiser->Read("Data", data, false);
                }

                resources.push_back(resource);
            }

            serialiser->StopArray();
        }
        else
        {
            u64 resourceSize = resources.size();
            serialiser->StartArray(c_Resources, resourceSize);
            
            for (Runtime::IResource* resource : resources)
            {
                Runtime::ResourceId resourceId = resource->GetResourceId();
                resourceId.Serialise(serialiser);
                resource->Serialise(serialiser);
                
                if(!resource->IsEngineFormat())
                {
                    std::vector<Byte> fileData = FileSystem::ReadFromFile(resource->GetFilePath(), FileType::Binary);

                    serialiser->Write("DataSize", fileData.size());
                    serialiser->Write("Data", fileData, false);
                }
            }
            
            serialiser->StopArray();
        }
    }
}