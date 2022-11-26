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
				m_mesh->OnUnloaded.Unbind<&MeshComponent::OnMaterialUnload>(this);
			}
			m_mesh = mesh;
		}

		void MeshComponent::SetMaterial(Runtime::Material* material)
		{
			if (m_material)
			{
				m_material->OnUnloaded.Unbind<&MeshComponent::OnMaterialUnload>(this);
			}
			m_material = material;
			m_material->OnUnloaded.Bind<&MeshComponent::OnMaterialUnload>(this);
		}

		void MeshComponent::OnMaterialUnload(Runtime::IResource* resource)
		{
			m_material->OnUnloaded.Unbind<&MeshComponent::OnMaterialUnload>(this);
			m_material = nullptr;
		}
	}
}