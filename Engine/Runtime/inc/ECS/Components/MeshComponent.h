#pragma once

#include "ECS/Entity.h"

#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/ResourceLifeTimeHandle.h"

namespace Insight
{
	namespace ECS
	{
		class MeshComponent : public Component
		{
		public:
			static constexpr char* Type_Name = "MeshComponent";
			virtual const char* GetTypeName() override { return Type_Name; }

			MeshComponent();
			virtual ~MeshComponent() override;

			void				SetMesh(Runtime::Mesh* mesh);
			Runtime::Mesh*		GetMesh()						const		{ return m_mesh; }
			void				SetMaterial(Runtime::Material* material);
			Runtime::Material*	GetMaterial()					const		{ return m_material; }

			IS_SERIALISABLE_H(MeshComponent)

		private:
			void OnMaterialUnload(Runtime::IResource* resource);

		private:
			Runtime::ResourceLFHandle<Runtime::Mesh> m_mesh;
			Runtime::ResourceLFHandle<Runtime::Material> m_material;
		};
	}

	namespace Serialisation
	{
		struct MeshToGuid {};
		template<>
		struct ComplexSerialiser<MeshToGuid, Runtime::ResourceLFHandle<Runtime::Mesh>, ECS::MeshComponent>
		{
			void operator()(ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Mesh>& mesh, ECS::MeshComponent* meshComponent) const;
		};
		struct MaterialToGuid {};
		template<>
		struct ComplexSerialiser<MaterialToGuid, Runtime::ResourceLFHandle<Runtime::Material>, ECS::MeshComponent>
		{
			void operator()(ISerialiser* serialiser, Runtime::ResourceLFHandle<Runtime::Material>& material, ECS::MeshComponent* meshComponent) const;
		};
	}

	OBJECT_SERIALISER(ECS::MeshComponent, 4,
		SERIALISE_BASE(ECS::Component, 2, 0)
		SERIALISE_COMPLEX(Serialisation::MeshToGuid, m_mesh, 3, 0)
		SERIALISE_COMPLEX(Serialisation::MaterialToGuid, m_material, 4, 0)
	);
}