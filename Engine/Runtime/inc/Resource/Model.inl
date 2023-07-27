#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<ModelMesh, std::vector<Runtime::Mesh*>, Runtime::Model>::operator()
            (ISerialiser* serialiser, std::vector<Runtime::Mesh*>& meshes, Runtime::Model* model) const
        {
            ASSERT(serialiser);
            constexpr const char* c_Meshes = "Meshes";
            if (serialiser->IsReadMode())
            {
                u64 arraySize = 0;
                serialiser->StartArray(c_Meshes, arraySize);
                meshes.resize(arraySize);

                for (Runtime::Mesh*& mesh : meshes)
                {
                    Runtime::ResourceId resourceId;
                    resourceId.Deserialise(serialiser);

                    Runtime::IResource* resource = Runtime::ResourceManager::Instance().Instance().CreateDependentResource(resourceId).Get();
                    ASSERT(resource);
                    model->AddDependentResource(resource);
                    mesh = static_cast<Runtime::Mesh*>(resource);
                    mesh->Deserialise(serialiser);

                    //Runtime::ResourceManager::Instance().UpdateGuidToResource(mesh);
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

        void ComplexSerialiser<ModelMaterial, std::vector<Runtime::Material*>, Runtime::Model>::operator()
            (ISerialiser* serialiser, std::vector<Runtime::Material*>& materials, Runtime::Model* model) const
        {
            ASSERT(serialiser);
            constexpr const char* c_Materials = "Materials";
            if (serialiser->IsReadMode())
            {
                u64 arraySize = 0;
                serialiser->StartArray(c_Materials, arraySize);
                materials.resize(arraySize);

                for (Runtime::Material*& material : materials)
                {
                    Runtime::ResourceId resourceId;
                    resourceId.Deserialise(serialiser);

                    ASSERT(!Runtime::ResourceManager::Instance().HasResource(resourceId));

                    Runtime::IResource* resource = Runtime::ResourceManager::Instance().Instance().CreateDependentResource(resourceId).Get();
                    ASSERT(resource);
                    model->AddDependentResource(resource);
                    material = static_cast<Runtime::Material*>(resource);
                    material->Deserialise(serialiser);

                    //Runtime::ResourceManager::Instance().UpdateGuidToResource(material);
                }
                serialiser->StopArray();
            }
            else
            {
                u64 arraySize = materials.size();
                serialiser->StartArray(c_Materials, arraySize);
                for (Runtime::Material* material : materials)
                {
                    material->GetResourceId().Serialise(serialiser);
                    material->Serialise(serialiser);
                }
                serialiser->StopArray();
            }
        }
    }
}