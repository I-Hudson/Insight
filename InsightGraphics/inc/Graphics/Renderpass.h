#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/ImGuiPass.h"

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
			void ImGuiPass();

			void UpdateCamera(UBO_Camera& camera);

			Mesh m_testMesh;

			Graphics::ImGuiPass m_imgui_pass;
			UBO_Camera m_camera;
			UBO_ShadowCamera m_shadowCamera;
		};
	}
}