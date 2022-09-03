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
			glm::mat4 Projection_View_Inverted;
		};

		struct UBO_ShadowCamera
		{
			glm::mat4 ProjView;
			glm::mat4 Projection;
			glm::mat4 View;
			glm::vec3 Light_Direction;
			float SplitDepth;
			glm::vec2 Resolution;
			float pad1;
			float pad2;

			static std::vector<UBO_ShadowCamera> GetCascades(const UBO_Camera& camera, int cascadeCount);
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
		};
	}
}