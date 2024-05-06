#pragma once

#include "Asset/Asset.h"
#include "Asset/Assets/Material.h"

#include "ECS/ICreateEntityHierarchy.h"

#include "Generated/Model_reflect_generated.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Buffer;
    }

    namespace Runtime
    {
        class ModelImporter;
        class Mesh;

        REFLECT_CLASS();
        class IS_RUNTIME ModelAsset : public Asset, public ECS::ICreateEntityHierarchy
        {
            REFLECT_GENERATED_BODY();
        public:
            ModelAsset(const AssetInfo* assetInfo);
            virtual ~ModelAsset() override;

            Mesh* GetMesh() const;
            Mesh* GetMeshByIndex(u32 index) const;
            u32 GetMeshCount() const;

            Ref<MaterialAsset> GetMaterial() const;
            Ref<MaterialAsset> GetMaterialByIndex(u32 index) const;
            u32 GetMaterialCount() const;

            //--ECS::ICreateEntityHierarchy
            virtual ECS::Entity* CreateEntityHierarchy() override;
            //--ECS::ICreateEntityHierarchy

            // BEGIN Asset
        protected:
            virtual void OnUnload() override;
            // END Asset

        private:
            std::vector<Mesh*> m_meshes;
            std::vector<Ref<MaterialAsset>> m_materials;

            Graphics::RHI_Buffer* m_vertex_buffer = nullptr;
            Graphics::RHI_Buffer* m_index_buffer = nullptr;

            friend class ModelImporter;
        };
    }
}