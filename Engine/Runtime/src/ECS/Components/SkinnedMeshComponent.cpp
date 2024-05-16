#include "ECS/Components/SkinnedMeshComponent.h"

namespace Insight
{
	namespace ECS
	{
		SkinnedMeshComponent::SkinnedMeshComponent()
		{ }
		SkinnedMeshComponent::~SkinnedMeshComponent()
		{ }

		IS_SERIALISABLE_CPP(SkinnedMeshComponent);

		void SkinnedMeshComponent::SetMesh(Ref<Runtime::Mesh> mesh)
		{
			m_mesh = mesh;
		}
		Ref<Runtime::Mesh> SkinnedMeshComponent::GetMesh() const
		{
			return m_mesh;
		}

		void SkinnedMeshComponent::SetMaterial(Ref<Runtime::MaterialAsset> material)
		{
			m_material = material;
		}
		Ref<Runtime::MaterialAsset> SkinnedMeshComponent::GetMaterial() const
		{
			return m_material;
		}

		void SkinnedMeshComponent::SetSkeleton(Ref<Runtime::Skeleton> skeleton)
		{
			m_skeleton = skeleton;
		}
		Ref<Runtime::Skeleton> SkinnedMeshComponent::GetSkeleton() const
		{
			return m_skeleton;
		}
	}
}