#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/MeshComponent.inl"

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
			m_mesh = mesh;
		}

		void MeshComponent::SetMaterial(Runtime::Material* material)
		{
			m_material = material;
		}
		
		IS_SERIALISABLE_CPP(MeshComponent)
	}
}