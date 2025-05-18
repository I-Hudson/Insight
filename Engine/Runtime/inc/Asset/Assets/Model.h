#pragma once

#include "Asset/Asset.h"
#include "Asset/Assets/Material.h"

#include "Resource/Mesh.h"
#include "Resource/Skeleton.h"
#include "Resource/AnimationClip.h"

#include "Maths/Vector2.h"

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

        REFLECT_CLASS();
        class IS_RUNTIME ModelAsset : public Asset, public ECS::ICreateEntityHierarchy
        {
            REFLECT_GENERATED_BODY();
        public:
            ModelAsset(const AssetInfo* assetInfo);
            virtual ~ModelAsset() override;

            Ref<Mesh> GetMesh() const;
            Ref<Mesh> GetMeshByIndex(const u32 index) const;
            u32 GetMeshCount() const;

            Ref<MaterialAsset> GetMaterial() const;
            Ref<MaterialAsset> GetMaterialByIndex(const u32 index) const;
            u32 GetMaterialCount() const;

            Ref<AnimationClip> GetAnimationByIndex(const u32 index) const;
            Ref<AnimationClip> GetAnimationByName(const std::string_view name) const;
            u32 GetAnimationCount() const;

            Ref<Skeleton> GetSkeleton(const u32 index) const;
            Ref<Skeleton> GetSkeletonByName(const std::string_view name) const;

            ECS::Entity* CreateEntityHierarchyStaticMesh() const;
            //--ECS::ICreateEntityHierarchy
            virtual ECS::Entity* CreateEntityHierarchy() const override;
            //--ECS::ICreateEntityHierarchy

            // BEGIN Asset
        protected:
            virtual void OnUnload() override;
            // END Asset

        private:
            std::vector<Ref<Mesh>> m_meshes;
            std::vector<Ref<MaterialAsset>> m_materials;
            std::vector<Ref<TextureAsset>> m_embeddedTextures;

            std::vector<Ref<Skeleton>> m_skeletons;
            std::vector<Ref<AnimationClip>> m_animationClips;

            Graphics::RHI_Buffer* m_vertex_buffer = nullptr;
            Graphics::RHI_Buffer* m_index_buffer = nullptr;

            friend class ModelImporter;
        };
    }
}