#pragma once

#include "Core/Singleton.h"

#include <glm/vec2.hpp>
#include <ffx-fsr2-api/ffx_fsr2.h>

namespace Insight
{
	namespace Graphics
	{
        class RenderContext;
        class RHI_CommandList;
        class RHI_Texture;

		class RHI_FSR : public Core::Singleton<RHI_FSR>
		{
		public:

            void Init();
            void Destroy();

            void GenerateJitterSample(float* x, float* y);
            void Dispatch(
                RHI_CommandList* cmd_list,
                RHI_Texture* tex_input,
                RHI_Texture* tex_depth,
                RHI_Texture* tex_velocity,
                RHI_Texture* tex_output,
                float camera_near_plane,
                float camera_far_plane,
                float camera_vertical_fov,
                float delta_time,
                float sharpness,
                bool reset
            );

        private:
            void CreateContext(u32 renderWidth, u32 renderHeight, u32 displayWidth, u32 displayHeight);

		private:
			FfxFsr2Context m_ffx_fsr2_context;
			FfxFsr2ContextDescription m_ffx_fsr2_context_description;
			FfxFsr2DispatchDescription m_ffx_fsr2_dispatch_description;
		};
	}
}