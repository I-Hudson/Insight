#pragma once

#include "Graphics/Mesh.h"

#include <glm/mat4x4.hpp>

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

		struct UBO_ShadowCamera : public UBO_Camera
		{
			glm::vec2 TextureSize;
		};

		class Renderpass
		{
		public:

			void Create();
			void Render();
			void Destroy();

		private:
			void ShadowPass();
			void Sample(UBO_Camera& camera);
			void Composite();
			void Swapchain();

			void UpdateCamera(UBO_Camera& camera);

			RenderTarget* m_shadowTarget;

			RenderTarget* m_depthTarget = nullptr;
			RenderTarget* m_colourTarget = nullptr;

			RenderTarget* m_compositeTarget = nullptr;

			Mesh m_testMesh;
			RHI_Texture* m_testTexture = nullptr;

			RHI_Buffer* m_vertexBuffer = nullptr;
			RHI_Buffer* m_indexBuffer = nullptr;

			UBO_Camera m_camera;
			UBO_ShadowCamera m_shadowCamera;
		};
	}
}