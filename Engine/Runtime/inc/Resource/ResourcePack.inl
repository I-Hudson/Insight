#include "Resource/ResourcePack.h"
#include "Resource/ResourceManager.h"

#include "FileSystem/FileSystem.h"

#include <zip.h>

namespace Insight::Serialisation
{
    void ComplexSerialiser<
        ResroucePack1
        , void
        , Runtime::ResourcePack>::
        operator()(ISerialiser* serialiser
            , Runtime::ResourcePack* resourcePack) const
    {
        if (serialiser != nullptr)
        {
            ASSERT(serialiser->GetType() == SerialisationTypes::Binary);
        }

        if (serialiser && serialiser->IsReadMode())
        {
        }
        else
        {
            const std::vector<Runtime::IResource*>& resources = resourcePack->m_resources;

            zip_t* zip = zip_stream_open(NULL, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
            for (Runtime::IResource* resource : resources)
            {
                if (resource->IsEngineFormat())
                {
                    zip_entry_open(zip, resource->GetFilePath().c_str());
                    ASSERT(zip_entry_fwrite(zip, resource->GetFilePath().c_str()) == 0);
                    zip_entry_close(zip);
                }
                else
                {
                    zip_entry_open(zip, resource->GetFilePath().c_str());
                    ASSERT(zip_entry_fwrite(zip, resource->GetFilePath().c_str()) == 0);
                    zip_entry_close(zip);

                    zip_entry_open(zip, Runtime::ResourceManager::GetMetaPath(resource).c_str());
                    ASSERT(zip_entry_fwrite(zip, Runtime::ResourceManager::GetMetaPath(resource).c_str()) == 0);
                    zip_entry_close(zip);
                }
            }

            char* zipData;
            size_t zipSize;
            zip_stream_copy(zip, (void**)&zipData, &zipSize);

            zip_stream_close(zip);

            // Serialise the zip file into the serialiser. If you want/need a resource pack to be saved on
            // it's own then you need to call the Save function.
            if (serialiser != nullptr)
            {
                serialiser->Write("zip", zipData, zipSize);
            }
            else
            {
                FileSystem::SaveToFile((Byte*)zipData, zipSize, resourcePack->GetFilePath(), true);
            }

            free(zipData);
        }
    }
}