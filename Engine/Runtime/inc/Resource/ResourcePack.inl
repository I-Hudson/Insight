#include "Resource/ResourcePack.h"

#include "Runtime/ProjectSystem.h"
#include "Serialisation/Archive.h"

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
        if (serialiser->IsReadMode())
        {
        }
        else
        {
            Serialisation::JsonSerialiser jsonSerialiser(false);
            SerialiserObject<Runtime::IResource> iResrouceObjectSerialiser;
            iResrouceObjectSerialiser.Serialise(&jsonSerialiser, *resourcePack);

            const std::vector<Runtime::IResource*> resources = resourcePack->m_resources;

            std::vector<std::string> filePaths;
            std::vector<const char*> filePathsCStr;

            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
            std::string metaFilePath = projectInfo.GetIntermediatePath() + "/ResourcePacks/" + resourcePack->GetFileName() + ".meta";
            
            Archive metaFile(metaFilePath, ArchiveModes::Write);
            metaFile.Write(jsonSerialiser.GetSerialisedData());
            metaFile.Close();

            filePaths.push_back(metaFilePath);
            for (const Runtime::IResource* res : resources)
            {
                filePaths.push_back(res->GetFilePath());
            }

            for (const std::string& str : filePaths)
            {
                filePathsCStr.push_back(str.c_str());
            }


            int result = zip_create(resourcePack->GetFilePath().c_str(), filePathsCStr.data(), filePathsCStr.size());
            ASSERT(result == 0);
        }
    }
}