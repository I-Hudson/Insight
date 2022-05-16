#pragma once

#include "Graphics/Mesh.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;

		struct UBO_Camera
		{
			glm::mat4 ProjView;
			glm::mat4 Projection;
			glm::mat4 View;
		};

		class Renderpass
		{
		public:

			void Create();
			void Render();
			void Destroy();

		private:
			void Sample();

			void UpdateCamera();

			Mesh m_testMesh;

			RHI_Buffer* m_vertexBuffer = nullptr;
			RHI_Buffer* m_indexBuffer = nullptr;

			UBO_Camera m_camera;
		};
	}
}