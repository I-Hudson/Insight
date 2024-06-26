#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/MeshComponent.inl"

#include "Resource/Mesh.h"

namespace Insight
{
	namespace ECS
	{
		MeshComponent::MeshComponent()
		{ }

		MeshComponent::~MeshComponent()
		{ }

		void MeshComponent::SetMesh(Ref<Runtime::Mesh> mesh)
		{
			m_mesh = mesh;
		}

		void MeshComponent::SetMaterial(Ref<Runtime::MaterialAsset> material)
		{
			m_material = material;
		}
		
		IS_SERIALISABLE_CPP(MeshComponent)
	}
}