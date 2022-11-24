#pragma once

#include "ECS/Entity.h"

namespace Insight
{
	namespace Runtime
	{
		class Mesh;
		class Material;
		class IResource;
	}

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

		private:
			void OnMeshUnload(Runtime::IResource* resource);
			void OnMaterialUnload(Runtime::IResource* resource);

		private:
			Runtime::Mesh* m_mesh = nullptr;
			Runtime::Material* m_material = nullptr;
		};
	}
}