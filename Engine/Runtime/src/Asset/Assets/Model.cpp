#include "Asset/Assets/Model.h"

#include "Core/Logger.h"

#include "Graphics/RenderContext.h"

#include "World/WorldSystem.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/SkinnedMeshComponent.h"
#include "ECS/Components/AnimationClipComponent.h"

namespace Insight::Runtime
{
	ModelAsset::ModelAsset(const AssetInfo* assetInfo)
		: Asset(assetInfo)
	{
	}

	ModelAsset::~ModelAsset()
	{
		OnUnload();
	}

	Ref<Mesh> ModelAsset::GetMesh() const
	{
		return GetMeshByIndex(0);
	}

	Ref<Mesh> ModelAsset::GetMeshByIndex(const u32 index) const
	{
		if (index >= static_cast<u32>(m_meshes.size()))
		{
			IS_LOG_CORE_WARN("[ModelAsset::GetMeshByIndex] Index '{}' is out of range.", index);
			return nullptr;
		}
		return m_meshes[index];
	}

	u32 ModelAsset::GetMeshCount() const
	{
		return static_cast<u32>(m_meshes.size());
	}

	Ref<MaterialAsset> ModelAsset::GetMaterial() const
	{
		return m_materials.empty() ? nullptr : m_materials[0];
	}

	Ref<MaterialAsset> ModelAsset::GetMaterialByIndex(const u32 index) const
	{
		if (index >= static_cast<u32>(m_materials.size()))
		{
			IS_LOG_CORE_WARN("[ModelAsset::GetMaterialByIndex] Index '{}' is out of range.", index);
			return Ref<MaterialAsset>();
		}
		return m_materials[index];
	}

	u32 ModelAsset::GetMaterialCount() const
	{
		return static_cast<u32>(m_materials.size());
	}

	Ref<AnimationClip> ModelAsset::GetAnimationByIndex(const u32 index) const
	{
		if (index >= static_cast<u32>(m_animationClips.size()))
		{
			IS_LOG_CORE_WARN("[ModelAsset::GetAnimationByIndex] Index '{}' is out of range.", index);
			return Ref<AnimationClip>();
		}
		return m_animationClips[index];
	}

	Ref<AnimationClip> ModelAsset::GetAnimationByName(const std::string_view name) const
	{
		for (size_t i = 0; i < m_animationClips.size(); ++i)
		{
			const Ref<AnimationClip>& clip = m_animationClips[i];
			if (clip->GetName() == name)
			{
				return clip;
			}
		}
		return Ref<AnimationClip>();
	}

	u32 ModelAsset::GetAnimationCount() const
	{
		return static_cast<u32>(m_animationClips.size());
	}

	Ref<Skeleton> ModelAsset::GetSkeleton(const u32 index) const
	{
		if (index >= static_cast<u32>(m_skeletons.size()))
		{
			IS_LOG_CORE_WARN("[ModelAsset::GetSkeleton] Index '{}' is out of range.", index);
			return Ref<Skeleton>();
		}
		return m_skeletons[index];
	}

	Ref<Skeleton> ModelAsset::GetSkeletonByName(const std::string_view name) const
	{
		for (size_t i = 0; i < m_skeletons.size(); ++i)
		{
			const Ref<Skeleton>& skeleton = m_skeletons[i];
			if (skeleton->GetName() == name)
			{
				return skeleton;
			}
		}
		return Ref<Skeleton>();
	}

	ECS::Entity* ModelAsset::CreateEntityHierarchyStaticMesh() const
	{
		TObjectPtr<World> world = Runtime::WorldSystem::Instance().GetActiveWorld();
		Ptr<ECS::Entity> root_entity = world->AddEntity(GetName());
		if (world)
		{
			for (const Ref<Mesh>& mesh : m_meshes)
			{
				Ptr<ECS::Entity> entity = root_entity->AddChild(std::string(mesh->GetName()));
				static_cast<ECS::TransformComponent*>(entity->GetComponentByName(ECS::TransformComponent::Type_Name))->SetTransform(mesh->GetTransform());
				ECS::MeshComponent* meshComponent = static_cast<ECS::MeshComponent*>(entity->AddComponentByName(ECS::MeshComponent::Type_Name));
				meshComponent->SetMesh(mesh);
				meshComponent->SetMaterial(mesh->GetMaterialAsset());
			}
		}
		return root_entity.Get();
	}

	ECS::Entity* ModelAsset::CreateEntityHierarchy() const
	{
		TObjectPtr<World> world = Runtime::WorldSystem::Instance().GetActiveWorld();

		if (world)
		{
			if (m_skeletons.size() > 0)
			{
				Ptr<ECS::Entity> root_entity = world->AddEntity(GetName());
				ECS::SkinnedMeshComponent* skinnedMeshComponent = static_cast<ECS::SkinnedMeshComponent*>(root_entity->AddComponentByName(ECS::SkinnedMeshComponent::Type_Name));
				skinnedMeshComponent->SetMesh(GetMesh());
				skinnedMeshComponent->SetMaterial(GetMesh()->GetMaterialAsset());
				skinnedMeshComponent->SetSkeleton(GetSkeleton(0));

				ECS::AnimationClipComponent* animationClipComponent = static_cast<ECS::AnimationClipComponent*>(root_entity->AddComponentByName(ECS::AnimationClipComponent::Type_Name));
				animationClipComponent->SetAnimationClip(m_animationClips[0]);
				animationClipComponent->SetSkeleton(GetSkeleton(0));
				return root_entity.Get();
			}
			else
			{
				return CreateEntityHierarchyStaticMesh();
			}
		}
		return nullptr;
	}

	void ModelAsset::OnUnload()
	{
		// Unload all our memory meshes.
		for (Ref<Mesh>& mesh : m_meshes)
		{
			mesh.Reset();
		}
		m_meshes.clear();

		for (Ref<MaterialAsset>& material : m_materials)
		{
			material->OnUnload();

			if (material->GetReferenceCount() > 1)
			{
				IS_LOG_CORE_WARN("[ModelAsset::OnUnload] Material '{}', is reference elsewhere. Will not be deleted here.", material->GetName());
			}
		}
		m_materials.clear();

		for (Ref<TextureAsset>& texture : m_embeddedTextures)
		{
			texture->OnUnload();

			if (texture->GetReferenceCount() > 1)
			{
				IS_LOG_CORE_WARN("[ModelAsset::OnUnload] Embedded Texture '{}', is reference elsewhere. Will not be deleted here.", texture->GetName());
			}
		}
		m_embeddedTextures.clear();
		

		for (Ref<AnimationClip>& animationClip : m_animationClips)
		{
			animationClip.Reset();
		}
		m_animationClips.clear();

		for (Ref<Skeleton>& skeleton : m_skeletons)
		{
			skeleton.Reset();
		}
		m_skeletons.clear();

		Renderer::FreeVertexBuffer(m_vertex_buffer);
		Renderer::FreeIndexBuffer(m_index_buffer);
		m_vertex_buffer = nullptr;
		m_index_buffer = nullptr;

		m_assetState = AssetState::Unloaded;
	}
}