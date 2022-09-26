#pragma once

#include "ECS/Entity.h"
#include "Graphics/Mesh.h"

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

			void SetMesh(Graphics::Mesh* mesh) { m_mesh = mesh; }
			Graphics::Mesh* GetMesh() const { return m_mesh; }

		private:
			Graphics::Mesh* m_mesh = nullptr;
		};
	}
}