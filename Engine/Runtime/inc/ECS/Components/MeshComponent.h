#pragma once

#include "ECS/Entity.h"

namespace Insight
{
	namespace Runtime
	{
		class Mesh;
		class Material;
	}

	namespace ECS
	{
		class MeshComponent : public Component
		{
		public:
			MeshComponent();
			~MeshComponent();

			static constexpr char* Type_Name = "MeshComponent";
			virtual const char* GetTypeName() override { return Type_Name; }

			void				SetMesh(Runtime::Mesh* mesh)				{ m_mesh = mesh; }
			Runtime::Mesh*		GetMesh()						const		{ return m_mesh; }
			void				SetMaterial(Runtime::Material* material)	{ m_material = material; }
			Runtime::Material*	GetMaterial()					const		{ return m_material; }

		private:
			Runtime::Mesh* m_mesh = nullptr;
			Runtime::Material* m_material = nullptr;
		};
	}
}