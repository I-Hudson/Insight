#include "ECS/Components/MeshComponent.h"

#include "Resource/Mesh.h"
#include "Resource/Material.h"

namespace Insight
{
	namespace ECS
	{
		MeshComponent::MeshComponent()
		{ }

		MeshComponent::~MeshComponent()
		{ }

		void MeshComponent::SetMesh(Runtime::Mesh* mesh)
		{
			if (m_mesh)
			{
				m_mesh->OnUnloaded.Unbind();
			}
			m_mesh = mesh;
			m_mesh->OnUnloaded.bind<MeshComponent, &MeshComponent::OnMeshUnload>(this);
		}

		void MeshComponent::SetMaterial(Runtime::Material* material)
		{
			if (m_material)
			{
				m_material->OnUnloaded.Unbind();
			}
			m_material = material;
			m_material->OnUnloaded.bind<MeshComponent, &MeshComponent::OnMaterialUnload>(this);
		}

		void MeshComponent::OnMeshUnload(Runtime::IResource* resource)
		{
			m_mesh->OnUnloaded.Unbind();
			m_mesh = nullptr;
		}

		void MeshComponent::OnMaterialUnload(Runtime::IResource* resource)
		{
			m_material->OnUnloaded.Unbind();
			m_material = nullptr;
		}
	}
}