#pragma once

#include "Graphics/ImGuiPass.h"
#include "Graphics/RHI/RHI_FSR.h"
#include "Graphics/RenderFrame.h"

#include "ECS/Components/CameraComponent.h"

#include "Core/TypeAlias.h"

#include <glm/mat4x4.hpp>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;
		class RHI_Sampler;
		class RenderTarget;

		struct BufferFrame
		{
			glm::mat4 Proj_View = { };
			glm::mat4 Projection = { };
			glm::mat4 View = { };
			glm::mat4 View_Inverted = { };
			glm::mat4 Projection_View_Inverted = { };



			glm::vec2 Render_Resolution = {};
			float pad0 = 0.0f;
			float pad1 = 0.0f;

			glm::vec2 Ouput_Resolution = {};
			float Delta_Time;
			
			float pad2 = 0.0f;
		};

		struct BufferSamplers
		{
			RHI_Sampler* Shadow_Sampler;
			RHI_Sampler* Repeat_Sampler;
			RHI_Sampler* Clamp_Sampler;
			RHI_Sampler* MirroredRepeat_Sampler;
		};

		static const u8 s_Cascade_Count = 4;
		struct BufferLight
		{
			glm::mat4 ProjView[s_Cascade_Count];
			glm::mat4 Projection[s_Cascade_Count];
			glm::mat4 View[s_Cascade_Count];
			float SplitDepth[s_Cascade_Count];

			glm::vec4 Light_Direction = { };
			glm::vec4 Light_Colour = glm::vec4(1, 1, 1, 1);
			glm::vec2 Resolution = { };

			float pad0 = 0.0f;
			float pad1 = 0.0f;

			static BufferLight GetCascades(const BufferFrame& buffer_frame, u32 cascade_count, float split_lambda = 0.95f);
			static void GetCascades(BufferLight& buffer_light, const BufferFrame& buffer_frame, u32 cascade_count, float split_lambda = 0.95f);
		};

		struct BufferPerObject
		{
			glm::mat4 Transform = {};
			glm::mat4 Previous_Transform = { };

			glm::vec4 Textures_Set;
		};

		class Renderpass
		{
		public:

			void Create();
			void Render();
			void Destroy();

		private:
			void ShadowPass();
			void ShadowCullingPass();
			void DepthPrepass();
			void GBuffer();
			void TransparentGBuffer();
			void Composite();
			void FSR2();
			void GFXHelper();
			void Swapchain();
			void ImGuiPass();

			void BindCommonResources(RHI_CommandList* cmd_list, BufferFrame& buffer_frame, BufferSamplers& buffer_samplers);

			Graphics::ImGuiPass m_imgui_pass;
			Graphics::RHI_FSR m_fsr;

			BufferFrame m_buffer_frame;
			BufferLight m_directional_light;
			BufferSamplers m_buffer_samplers;

			ECS::Entity* m_editorCameraEntity;
			ECS::CameraComponent* m_editorCameraComponent;

			float m_taaJitterX;
			float m_taaJitterY;
		};
	}
}