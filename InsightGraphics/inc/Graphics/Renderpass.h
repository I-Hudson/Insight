#pragma once

#include "Graphics/Mesh.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;

		class Renderpass
		{
		public:
			void Create();
			void Render();
			void Destroy();

		private:
			void Sample();

			Mesh m_testMesh;

			RHI_Buffer* m_vertexBuffer = nullptr;
			RHI_Buffer* m_indexBuffer = nullptr;
		};
	}
}