#pragma once

#include "Runtime/Defines.h"
#include "Graphics/ImGuiPass.h"
#include "Graphics/RHI/RHI_FSR.h"
#include "Graphics/RenderFrame.h"

#include "ECS/Components/CameraComponent.h"

#include "Maths/Vector2.h"
#include "Maths/Vector4.h"

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
			Maths::Matrix4 Proj_View = { };
			Maths::Matrix4 Projection = { };
			Maths::Matrix4 View = { };
			Maths::Matrix4 View_Inverted = { };
			Maths::Matrix4 Projection_View_Inverted = { };

			glm::vec2 Render_Resolution = {};
			glm::vec2 Ouput_Resolution = {};

			glm::vec2 TAA_Jitter_Current;
			glm::vec2 TAA_Jitter_Previous;

			float Delta_Time;
			float __pad0;
			float __pad1;
			float __pad2;
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
			Maths::Matrix4 ProjView[s_Cascade_Count];
			Maths::Matrix4 Projection[s_Cascade_Count];
			Maths::Matrix4 View[s_Cascade_Count];
			float SplitDepth[s_Cascade_Count];

			Maths::Vector4 Light_Direction = { };
			Maths::Vector4 Light_Colour = Maths::Vector4(1, 1, 1, 1);
			Maths::Vector2 Resolution = { };

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

		class IS_RUNTIME Renderpass
		{
		public:
			void Create();
			void Render(bool render = true);
			void Destroy();

			void FrameSetup();
			void RenderMainPasses(bool render);
			void RenderSwapchain(bool renderResultImage);
			void RenderPostprocessing();

		private:
			void ShadowPass();
			void DepthPrepass();
			void GBuffer();
			void TransparentGBuffer();
			void Composite();
			void FSR2();
			void GFXHelper();
			void Swapchain(bool renderResultImage);
			void ImGuiPass();

			void CreateAllCommonShaders();
			void BindCommonResources(RHI_CommandList* cmd_list, BufferFrame& buffer_frame, BufferSamplers& buffer_samplers);

			Graphics::ImGuiPass m_imgui_pass;
			Graphics::RHI_FSR m_fsr;

			BufferFrame m_buffer_frame;
			BufferLight m_directional_light;
			BufferSamplers m_buffer_samplers;

			float m_taaJitterX;
			float m_taaJitterY;
		};
	}
}