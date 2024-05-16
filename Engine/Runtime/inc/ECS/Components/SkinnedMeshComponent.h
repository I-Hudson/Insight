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
		class IS_RUNTIME SkinnedMeshComponent : public Component
		{
			REFLECT_GENERATED_BODY();
		public:
			IS_COMPONENT(SkinnedMeshComponent);

			SkinnedMeshComponent();
			virtual ~SkinnedMeshComponent() override;

			IS_SERIALISABLE_H(SkinnedMeshComponent);

			void SetMesh(Ref<Runtime::Mesh> mesh);
			Ref<Runtime::Mesh> GetMesh() const;

			void SetMaterial(Ref<Runtime::MaterialAsset> material);
			Ref<Runtime::MaterialAsset> GetMaterial() const;

			void SetSkeleton(Ref<Runtime::Skeleton> skeleton);
			Ref<Runtime::Skeleton> GetSkeleton() const;

		private:
			Ref<Runtime::Mesh> m_mesh;
			Ref<Runtime::MaterialAsset> m_material;
			Ref<Runtime::Skeleton> m_skeleton;
		};
	}

	OBJECT_SERIALISER(ECS::SkinnedMeshComponent, 1,
		SERIALISE_BASE(ECS::Component, 1, 0)
		//SERIALISE_COMPLEX(Serialisation::MeshToGuid, m_mesh, 1, 0)
		//SERIALISE_COMPLEX(Serialisation::MaterialToGuid, m_material, 1, 0)
	);
}