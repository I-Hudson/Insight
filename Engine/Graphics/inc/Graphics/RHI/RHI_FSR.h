#pragma once

#include "Graphics/Defines.h"

#include "Core/Singleton.h"
#include "Core/TypeAlias.h"

#include <glm/vec2.hpp>

struct FfxFsr2Context;
struct FfxFsr2ContextDescription;
struct FfxFsr2DispatchDescription;

namespace Insight
{
	namespace Graphics
	{
        class RenderContext;
        class RHI_CommandList;
        class RHI_Texture;

		class IS_GRAPHICS RHI_FSR : public Core::Singleton<RHI_FSR>
		{
		public:

            void Init();
            void Destroy();

            void SetIsEnabled(const bool value) const;
            bool IsEnabled() const;

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
			static FfxFsr2Context m_ffx_fsr2_context;
			static FfxFsr2ContextDescription m_ffx_fsr2_context_description;
			static FfxFsr2DispatchDescription m_ffx_fsr2_dispatch_description;

            static bool m_fsr2IsEnabled;
		};
	}
}