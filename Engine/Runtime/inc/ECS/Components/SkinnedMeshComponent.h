#pragma once

#include "ECS/Entity.h"
#include "ECS/Components/MeshComponent.h"

#include "Asset/Assets/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Skeleton.h"

#include "Generated/SkinnedMeshComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS();
		class IS_RUNTIME SkinnedMeshComponent : public MeshComponent
		{
			REFLECT_GENERATED_BODY();
		public:
			IS_COMPONENT(SkinnedMeshComponent);

			SkinnedMeshComponent();
			virtual ~SkinnedMeshComponent() override;

			IS_SERIALISABLE_H(SkinnedMeshComponent);

			void SetSkeleton(Ref<Runtime::Skeleton> skeleton);
			Ref<Runtime::Skeleton> GetSkeleton() const;

		private:
			Ref<Runtime::Skeleton> m_skeleton;
		};
	}

	namespace Serialisation
	{
		struct SkeletonToGuid {};
		template<>
		struct ComplexSerialiser<SkeletonToGuid, Ref<Runtime::Skeleton>, ECS::SkinnedMeshComponent>
		{
			void operator()(ISerialiser* serialiser, Ref<Runtime::Skeleton>& skelton, ECS::SkinnedMeshComponent* component) const;
		};
	}

	OBJECT_SERIALISER(ECS::SkinnedMeshComponent, 2,
		SERIALISE_BASE_REMOVED(ECS::Component, 1, 2)
		SERIALISE_BASE(ECS::MeshComponent, 2, 0)
		SERIALISE_COMPLEX(Serialisation::SkeletonToGuid, m_skeleton, 2, 0)
	);
}