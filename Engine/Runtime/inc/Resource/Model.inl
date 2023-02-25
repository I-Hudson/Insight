#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<ModelMesh, std::vector<Runtime::Mesh*>, Runtime::Model>::operator()
            (ISerialiser* serialiser, std::vector<Runtime::Mesh*>& meshes, Runtime::Model* model) const
        {
            ASSERT(serialiser);
            constexpr char* c_Meshes = "Meshes";
            if (serialiser->IsReadMode())
            {
                u64 arraySize = 0;
                serialiser->StartArray(c_Meshes, arraySize);
                meshes.resize(arraySize);

                for (Runtime::Mesh*& mesh : meshes)
                {
                    Runtime::ResourceId resourceId;
                    resourceId.Deserialise(serialiser);

                    Runtime::IResource* resource = Runtime::ResourceManager::CreateDependentResource(resourceId).Get();
                    ASSERT(resource);

                    mesh = static_cast<Runtime::Mesh*>(resource);
                    mesh->Deserialise(serialiser);
                }

                serialiser->StopArray();
            }
            else
            {
                u64 arraySize = meshes.size();
                serialiser->StartArray(c_Meshes, arraySize);
                for (Runtime::Mesh* mesh : meshes)
                {
                    mesh->GetResourceId().Serialise(serialiser);
                    mesh->Serialise(serialiser);
                }
                serialiser->StopArray();
            }
        }
    }
}