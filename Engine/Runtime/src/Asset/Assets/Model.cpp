#include "Asset/Assets/Model.h"

#include "Graphics/RenderContext.h"

#include "World/WorldSystem.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/MeshComponent.h"

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

	Mesh* ModelAsset::GetMesh() const
	{
		return GetMeshByIndex(0);
	}

	Mesh* ModelAsset::GetMeshByIndex(u32 index) const
	{
		return m_meshes.at(index);
	}

	u32 ModelAsset::GetMeshCount() const
	{
		return static_cast<u32>(m_meshes.size());
	}

	Material* ModelAsset::GetMaterial() const
	{
		return m_materials.empty() ? nullptr : m_materials.at(0);
	}

	Material* ModelAsset::GetMaterialByIndex(u32 index) const
	{
		return m_materials.at(index);
	}

	u32 ModelAsset::GetMaterialCount() const
	{
		return static_cast<u32>(m_materials.size());
	}

	ECS::Entity* ModelAsset::CreateEntityHierarchy()
	{
		TObjectPtr<World> world = Runtime::WorldSystem::Instance().GetActiveWorld();
		if (world)
		{
			Ptr<ECS::Entity> root_entity = world->AddEntity(GetFileName());
			for (Mesh* mesh : m_meshes)
			{
				Ptr<ECS::Entity> entity = root_entity->AddChild(mesh->GetFileName());
				static_cast<ECS::TransformComponent*>(entity->GetComponentByName(ECS::TransformComponent::Type_Name))->SetTransform(mesh->GetTransform());
				ECS::MeshComponent* meshComponent = static_cast<ECS::MeshComponent*>(entity->AddComponentByName(ECS::MeshComponent::Type_Name));
				meshComponent->SetMesh(mesh);
				meshComponent->SetMaterial(mesh->GetMaterial());
			}
			return root_entity.Get();
		}
		return nullptr;
	}

	void ModelAsset::OnUnload()
	{
		// Unload all our memory meshes.
		for (Mesh* mesh : m_meshes)
		{
			mesh->OnUnloaded(mesh);
			//RemoveDependentResource(mesh);
			//ResourceManager::Instance().RemoveDependentResource(mesh->GetResourceId());
		}
		m_meshes.clear();

		int materialIdx = 0;
		for (Material* material : m_materials)
		{
			material->OnUnloaded(material);
			//RemoveDependentResource(material);
			//ResourceManager::Instance().RemoveDependentResource(material->GetResourceId());
			++materialIdx;
		}
		m_materials.clear();

		Renderer::FreeVertexBuffer(m_vertex_buffer);
		Renderer::FreeIndexBuffer(m_index_buffer);
		m_vertex_buffer = nullptr;
		m_index_buffer = nullptr;

		m_assetState = AssetState::Unloaded;
	}
}