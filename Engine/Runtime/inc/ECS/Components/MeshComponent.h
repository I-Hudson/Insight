#pragma once

#include "ECS/Entity.h"
#include "Resource/Mesh.h"

namespace Insight
{
	namespace ECS
	{
		class MeshComponent : public Component
		{
		public:
			MeshComponent();
			~MeshComponent();

			static constexpr char* Type_Name = "MeshComponent";
			virtual const char* GetTypeName() override { return Type_Name; }

			void SetMesh(Runtime::Mesh* mesh) { m_mesh = mesh; }
			Runtime::Mesh* GetMesh() const { return m_mesh; }

		private:
			Runtime::Mesh* m_mesh = nullptr;
		};
	}
}