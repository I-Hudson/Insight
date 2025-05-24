#include "ECS/Components/SkinnedMeshComponent.h"
#include "ECS/Components/SkinnedMeshComponent.inl"

namespace Insight
{
	namespace ECS
	{
		SkinnedMeshComponent::SkinnedMeshComponent()
		{ }
		SkinnedMeshComponent::~SkinnedMeshComponent()
		{ }

		IS_SERIALISABLE_CPP(SkinnedMeshComponent);

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