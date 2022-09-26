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
			glm::mat4 View_Inverted;
			glm::mat4 Projection_View_Inverted;
		};

		struct UBO_ShadowCamera
		{
			glm::mat4 ProjView;		
			glm::mat4 Projection;	
			glm::mat4 View;			
			glm::vec4 Light_Direction;
			glm::vec2 Resolution;	
			float SplitDepth;
			float pad0 = -1.0f;

			static std::vector<UBO_ShadowCamera> GetCascades(const UBO_Camera& camera, int cascadeCount, float split_lambda = 0.95f);
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