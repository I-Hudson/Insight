#include "Graphics/RHI/RHI_FSR.h"
#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/RHI_Texture.h"

#include "Graphics/GraphicsManager.h"

#include "Platform/Platform.h"

#include "Event/EventManager.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#endif
#ifdef IS_DX12_ENABLED
#endif

#include <ffx-fsr2-api/vk/ffx_fsr2_vk.h>

namespace Insight
{
	namespace Graphics
	{
        void RHI_FSR::Init()
        {
            Core::EventManager::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& event)
                {
                    RenderContext* render_context = GraphicsManager::Instance().GetRenderContext();

                   
                });
        }

        void RHI_FSR::Destroy()
        {
            Core::EventManager::Instance().RemoveEventListener(this, Core::EventType::Graphics_Swapchain_Resize);
            ASSERT(ffxFsr2ContextDestroy(&m_ffx_fsr2_context) == FFX_OK);
        }

		void RHI_FSR::GenerateJitterSample(float* x, float* y)
		{
			// Get render and output resolution from the context description (safe to do as we are not using dynamic resolution)
			u32 resolution_render_x = static_cast<uint32_t>(m_ffx_fsr2_context_description.maxRenderSize.width);
            u32 resolution_output_x = static_cast<uint32_t>(m_ffx_fsr2_context_description.displaySize.width);

			// Generate jitter sample
			static uint32_t index = 0; index++;
			const int32_t jitter_phase_count = ffxFsr2GetJitterPhaseCount(resolution_render_x, resolution_output_x);
			ASSERT(ffxFsr2GetJitterOffset(&m_ffx_fsr2_dispatch_description.jitterOffset.x, &m_ffx_fsr2_dispatch_description.jitterOffset.y, index, jitter_phase_count) == FFX_OK);

			// Out
			*x = m_ffx_fsr2_dispatch_description.jitterOffset.x;
			*y = m_ffx_fsr2_dispatch_description.jitterOffset.y;
		}

		void RHI_FSR::Dispatch(RHI_CommandList* cmd_list, RHI_Texture* tex_input, RHI_Texture* tex_depth, RHI_Texture* tex_velocity, RHI_Texture* tex_output, float camera_near_plane, float camera_far_plane, float camera_vertical_fov, float delta_time, float sharpness, bool reset)
		{
            // Get render and output resolution from the context description (safe to do as we are not using dynamic resolution)
            uint32_t resolution_render_x = static_cast<uint32_t>(m_ffx_fsr2_context_description.maxRenderSize.width);
            uint32_t resolution_render_y = static_cast<uint32_t>(m_ffx_fsr2_context_description.maxRenderSize.height);
            uint32_t resolution_output_x = static_cast<uint32_t>(m_ffx_fsr2_context_description.displaySize.width);
            uint32_t resolution_output_y = static_cast<uint32_t>(m_ffx_fsr2_context_description.displaySize.height);

            // Define texture names
            wchar_t name_input[] = L"FSR2_Input";
            wchar_t name_depth[] = L"FSR2_Depth";
            wchar_t name_velocity[] = L"FSR2_Velocity";
            wchar_t name_exposure[] = L"FSR2_Exposure";
            wchar_t name_output[] = L"FSR2_Output";

            // Transition to the appropriate texture layouts (will only happen if needed)
            cmd_list->SetImageLayout(tex_input, ImageLayout::ShaderReadOnly);
            cmd_list->SetImageLayout(tex_depth, ImageLayout::ShaderReadOnly);
            cmd_list->SetImageLayout(tex_velocity, ImageLayout::ShaderReadOnly);
            cmd_list->SetImageLayout(tex_output, ImageLayout::General);

            // Fill in the dispatch description
            m_ffx_fsr2_dispatch_description = {};
            if (GraphicsManager::Instance().IsVulkan())
            {
                m_ffx_fsr2_dispatch_description.commandList   = ffxGetCommandListVK(static_cast<RHI::Vulkan::RHI_CommandList_Vulkan*>(cmd_list)->GetCommandList().operator VkCommandBuffer());

                m_ffx_fsr2_dispatch_description.color         = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImageView(), resolution_render_x, resolution_render_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_input->GetFormat())), name_input, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.depth         = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_depth)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImageView(), resolution_render_x, resolution_render_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_depth->GetFormat())), name_depth, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.motionVectors = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_velocity)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImageView(), resolution_render_x, resolution_render_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_velocity->GetFormat())), name_velocity, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.output        = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_output)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImageView(), resolution_output_x, resolution_output_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_output->GetFormat())), name_output, FFX_RESOURCE_STATE_UNORDERED_ACCESS);

            }
            else if (GraphicsManager::Instance().IsDX12())
            {

            }
            m_ffx_fsr2_dispatch_description.jitterOffset.x          = 0;
            m_ffx_fsr2_dispatch_description.jitterOffset.y          = 0;
            m_ffx_fsr2_dispatch_description.motionVectorScale.y     = -static_cast<float>(resolution_render_y);
            m_ffx_fsr2_dispatch_description.motionVectorScale.y     = -static_cast<float>(resolution_render_y);
            m_ffx_fsr2_dispatch_description.reset                   = reset;                                    // A boolean value which when set to true, indicates the camera has moved discontinuously.
            m_ffx_fsr2_dispatch_description.enableSharpening        = sharpness != 0.0f;
            m_ffx_fsr2_dispatch_description.sharpness               = sharpness;
            m_ffx_fsr2_dispatch_description.frameTimeDelta          = delta_time;                               // Seconds to milliseconds.
            m_ffx_fsr2_dispatch_description.preExposure             = 1.0f;                                     // The exposure value if not using FFX_FSR2_ENABLE_AUTO_EXPOSURE.
            m_ffx_fsr2_dispatch_description.renderSize.width        = resolution_render_x;
            m_ffx_fsr2_dispatch_description.renderSize.height       = resolution_render_y;
            m_ffx_fsr2_dispatch_description.cameraNear              = camera_near_plane;
            m_ffx_fsr2_dispatch_description.cameraFar               = camera_far_plane;
            m_ffx_fsr2_dispatch_description.cameraFovAngleVertical  = camera_vertical_fov;

            ASSERT(ffxFsr2ContextDispatch(&m_ffx_fsr2_context, &m_ffx_fsr2_dispatch_description) == FFX_OK);
		}

        void RHI_FSR::CreateContext(u32 renderWidth, u32 renderHeight, u32 displayWidth, u32 displayHeight)
        {
            RenderContext* render_context = GraphicsManager::Instance().GetRenderContext();
            if (GraphicsManager::IsVulkan())
            {
#ifdef IS_VULKAN_ENABLED
                RHI::Vulkan::RenderContext_Vulkan* renderContextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(render_context);
                const u64 scratchBufferSize = ffxFsr2GetScratchMemorySizeVK(renderContextVulkan->GetPhysicalDevice());
                void* scratchBuffer = malloc(scratchBufferSize);
                FfxErrorCode errorCode = ffxFsr2GetInterfaceVK(&m_ffx_fsr2_context_description.callbacks, scratchBuffer
                    , scratchBufferSize
                    , renderContextVulkan->GetPhysicalDevice()
                    , vkGetDeviceProcAddr);
                ASSERT(errorCode == FFX_OK);

                m_ffx_fsr2_context_description.device = ffxGetDeviceVK(renderContextVulkan->GetDevice());
                m_ffx_fsr2_context_description.maxRenderSize.width = renderWidth;
                m_ffx_fsr2_context_description.maxRenderSize.height = renderHeight;
                m_ffx_fsr2_context_description.displaySize.width = displayWidth;
                m_ffx_fsr2_context_description.displaySize.height = displayHeight;
                m_ffx_fsr2_context_description.flags = FFX_FSR2_ENABLE_AUTO_EXPOSURE;

                ffxFsr2ContextCreate(&m_ffx_fsr2_context, &m_ffx_fsr2_context_description);
#endif
            }
            if (GraphicsManager::IsDX12())
            {
#ifdef IS_DX12_ENABLED
#endif
            }
        }
	}
}