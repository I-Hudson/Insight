#pragma once

#include "Graphics/Mesh.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;
		class RenderTarget;

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
			void ShadowPass();
			void Sample();

			void UpdateCamera();

			RenderTarget* m_shadowTarget;

			RenderTarget* m_depthTarget = nullptr;
			RenderTarget* m_colourTarget = nullptr;

			Mesh m_testMesh;
			RHI_Texture* m_testTexture = nullptr;

			RHI_Buffer* m_vertexBuffer = nullptr;
			RHI_Buffer* m_indexBuffer = nullptr;

			UBO_Camera m_camera;
		};
	}
}