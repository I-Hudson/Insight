#pragma once

#include "Asset/Asset.h"
#include "Resource/Material.h"
#include "Resource/Mesh.h"

#include "ECS/ICreateEntityHierarchy.h"

#include "Generated/Model_reflect_generated.h"

namespace Insight::Runtime
{
    class ModelImporter;

    REFLECT_CLASS()
    class ModelAsset : public Asset, public ECS::ICreateEntityHierarchy
    {
        REFLECT_GENERATED_BODY()
    public:
        ModelAsset(const AssetInfo* assetInfo);
        virtual ~ModelAsset() override;

        Mesh* GetMesh() const;
        Mesh* GetMeshByIndex(u32 index) const;
        u32 GetMeshCount() const;

        Material* GetMaterial() const;
        Material* GetMaterialByIndex(u32 index) const;
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
        std::vector<Material*> m_materials;

        Graphics::RHI_Buffer* m_vertex_buffer;
        Graphics::RHI_Buffer* m_index_buffer;

        friend class ModelImporter;
    };
}