#pragma once

#include "ECS/Entity.h"

#include "Resource/Mesh.h"

#include "Generated/MeshComponent_reflect_generated.h"

namespace Insight
{
	namespace ECS
	{
		REFLECT_CLASS()
		class IS_RUNTIME MeshComponent : public Component
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(MeshComponent);

			MeshComponent();
			virtual ~MeshComponent() override;

			void						SetMesh(Runtime::Mesh* mesh);
			Runtime::Mesh*				GetMesh()						const		{ return m_mesh; }
			void						SetMaterial(Ref<Runtime::MaterialAsset> material);
			Ref<Runtime::MaterialAsset>	GetMaterial()					const		{ return m_material; }

			IS_SERIALISABLE_H(MeshComponent)

		private:
			Runtime::Mesh* m_mesh;
			Ref<Runtime::MaterialAsset> m_material;
		};
	}

	namespace Serialisation
	{
		struct MeshToGuid {};
		template<>
		struct ComplexSerialiser<MeshToGuid, Runtime::Mesh*, ECS::MeshComponent>
		{
			void operator()(ISerialiser* serialiser, Runtime::Mesh*& mesh, ECS::MeshComponent* meshComponent) const;
		};
		struct MaterialToGuid {};
		template<>
		struct ComplexSerialiser<MaterialToGuid, Ref<Runtime::MaterialAsset>, ECS::MeshComponent>
		{
			void operator()(ISerialiser* serialiser, Ref<Runtime::MaterialAsset>& material, ECS::MeshComponent* meshComponent) const;
		};
	}

	OBJECT_SERIALISER(ECS::MeshComponent, 4,
		SERIALISE_BASE(ECS::Component, 2, 0)
		SERIALISE_COMPLEX(Serialisation::MeshToGuid, m_mesh, 3, 0)
		SERIALISE_COMPLEX(Serialisation::MaterialToGuid, m_material, 4, 0)
	);
}