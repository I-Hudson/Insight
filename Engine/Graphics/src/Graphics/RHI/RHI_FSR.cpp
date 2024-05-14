#include "Graphics/RHI/RHI_FSR.h"
#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/RHI_Texture.h"

#include "Core/Asserts.h"

#include "Event/EventSystem.h"

#include <ffx-fsr2-api/ffx_fsr2.h>

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include <ffx-fsr2-api/vk/ffx_fsr2_vk.h>
#endif
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include <ffx-fsr2-api/dx12/ffx_fsr2_dx12.h>
#endif


namespace Insight
{
	namespace Graphics
	{
        FfxFsr2Context              RHI_FSR::m_ffx_fsr2_context;
        FfxFsr2ContextDescription   RHI_FSR::m_ffx_fsr2_context_description;
        FfxFsr2DispatchDescription  RHI_FSR::m_ffx_fsr2_dispatch_description;
        bool                        RHI_FSR::m_fsr2IsEnabled;

        void RHI_FSR::Init()
        {
            Maths::Vector2 render_resolution = RenderGraph::Instance().GetRenderResolution();
            Maths::Vector2 output_resolution = RenderGraph::Instance().GetOutputResolution();
            CreateContext(render_resolution.x, render_resolution.y, output_resolution.x, output_resolution.y);

            //Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& event)
            //    {
            //        if (m_ffx_fsr2_context_description.callbacks.scratchBuffer != nullptr)
            //        {
            //            ASSERT(ffxFsr2ContextDestroy(&m_ffx_fsr2_context) == FFX_OK);
            //            free(m_ffx_fsr2_context_description.callbacks.scratchBuffer);
            //            m_ffx_fsr2_context_description.callbacks.scratchBuffer = nullptr;
            //        }

            //        RenderContext* render_context = &RenderContext::Instance();
            //        Maths::Vector2 render_resolution = RenderGraph::Instance().GetRenderResolution();
            //        Maths::Vector2 output_resolution = RenderGraph::Instance().GetOutputResolution();
            //        CreateContext(render_resolution.x, render_resolution.y, output_resolution.x, output_resolution.y);
            //    });
            Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Graphics_Render_Resolution_Change, [this](const Core::Event& event)
                {
                    GPUDeferedManager::Instance().Push([this](RHI_CommandList* cmdList)
                        {
                            RenderContext::Instance().GpuWaitForIdle();
                            if (m_ffx_fsr2_context_description.callbacks.scratchBuffer != nullptr)
                            {
                                ASSERT(ffxFsr2ContextDestroy(&m_ffx_fsr2_context) == FFX_OK);
                                free(m_ffx_fsr2_context_description.callbacks.scratchBuffer);
                                m_ffx_fsr2_context_description.callbacks.scratchBuffer = nullptr;
                            }

                            RenderContext* render_context = &RenderContext::Instance();
                            Maths::Vector2 render_resolution = RenderGraph::Instance().GetRenderResolution();
                            Maths::Vector2 output_resolution = RenderGraph::Instance().GetOutputResolution();
                            CreateContext(render_resolution.x, render_resolution.y, output_resolution.x, output_resolution.y);
                        });
                });
        }

        void RHI_FSR::Destroy()
        {
            Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Graphics_Swapchain_Resize);
            if (m_ffx_fsr2_context_description.callbacks.scratchBuffer != nullptr)
            {
                ASSERT(ffxFsr2ContextDestroy(&m_ffx_fsr2_context) == FFX_OK);
                free(m_ffx_fsr2_context_description.callbacks.scratchBuffer);
                m_ffx_fsr2_context_description.callbacks.scratchBuffer = nullptr;
            }
        }

        void RHI_FSR::SetIsEnabled(const bool value) const
        {
            m_fsr2IsEnabled = value;
        }

        bool RHI_FSR::IsEnabled() const
        {
            return m_fsr2IsEnabled;
        }
           
		void RHI_FSR::GenerateJitterSample(float* x, float* y)
		{
			// Get render and output resolution from the context description (safe to do as we are not using dynamic resolution)
            u32 resolution_render_x = static_cast<uint32_t>(m_ffx_fsr2_context_description.maxRenderSize.width);
            u32 resolution_render_y = static_cast<uint32_t>(m_ffx_fsr2_context_description.maxRenderSize.height);

            u32 resolution_output_x = static_cast<uint32_t>(m_ffx_fsr2_context_description.displaySize.width);

			// Generate jitter sample
			static uint32_t index = 0; index++;
			const int32_t jitter_phase_count = ffxFsr2GetJitterPhaseCount(resolution_render_x, resolution_output_x);
            FfxErrorCode jitterOffsetErrorCode = ffxFsr2GetJitterOffset(&m_ffx_fsr2_dispatch_description.jitterOffset.x, &m_ffx_fsr2_dispatch_description.jitterOffset.y, index, jitter_phase_count);
            ASSERT(jitterOffsetErrorCode == FFX_OK);

            m_ffx_fsr2_dispatch_description.jitterOffset.x = (m_ffx_fsr2_dispatch_description.jitterOffset.x / resolution_render_x);
            m_ffx_fsr2_dispatch_description.jitterOffset.y = (m_ffx_fsr2_dispatch_description.jitterOffset.y / resolution_render_y);

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
            //m_ffx_fsr2_dispatch_description = {};
            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
            {
#ifdef IS_VULKAN_ENABLED
                m_ffx_fsr2_dispatch_description.commandList   = ffxGetCommandListVK(static_cast<RHI::Vulkan::RHI_CommandList_Vulkan*>(cmd_list)->GetCommandList());

                m_ffx_fsr2_dispatch_description.color         = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_input)->GetImageView(), resolution_render_x, resolution_render_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_input->GetFormat())), name_input, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.depth         = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_depth)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_depth)->GetImageView(), resolution_render_x, resolution_render_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_depth->GetFormat())), name_depth, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.motionVectors = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_velocity)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_velocity)->GetImageView(), resolution_render_x, resolution_render_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_velocity->GetFormat())), name_velocity, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.output        = ffxGetTextureResourceVK(&m_ffx_fsr2_context, static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_output)->GetImage()
                    , static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(tex_output)->GetImageView(), resolution_output_x, resolution_output_y
                    , static_cast<VkFormat>(PixelFormatToVulkan(tex_output->GetFormat())), name_output, FFX_RESOURCE_STATE_UNORDERED_ACCESS);

                //m_ffx_fsr2_dispatch_description.exposure                    = ffxGetTextureResourceVK(&m_ffx_fsr2_context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, L"FSR2_InputExposure");
                //m_ffx_fsr2_dispatch_description.reactive                    = ffxGetTextureResourceVK(&m_ffx_fsr2_context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, L"FSR2_EmptyInputReactiveMap");
                //m_ffx_fsr2_dispatch_description.transparencyAndComposition  = ffxGetTextureResourceVK(&m_ffx_fsr2_context, nullptr, nullptr, 1, 1, VK_FORMAT_UNDEFINED, L"FSR2_EmptyTransparencyAndCompositionMap");            
#endif
            }
            else if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
            {
#ifdef IS_DX12_ENABLED
                RHI::DX12::RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI::DX12::RHI_CommandList_DX12*>(cmd_list);
                cmdListDX12->m_boundResourceHeap = nullptr;
                m_ffx_fsr2_dispatch_description.commandList = ffxGetCommandListDX12(cmdListDX12->GetCommandList());

                m_ffx_fsr2_dispatch_description.color = ffxGetResourceDX12(&m_ffx_fsr2_context, static_cast<RHI::DX12::RHI_Texture_DX12*>(tex_input)->GetResource()
                    , name_input, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.depth = ffxGetResourceDX12(&m_ffx_fsr2_context, static_cast<RHI::DX12::RHI_Texture_DX12*>(tex_depth)->GetResource()
                    , name_depth, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.motionVectors = ffxGetResourceDX12(&m_ffx_fsr2_context, static_cast<RHI::DX12::RHI_Texture_DX12*>(tex_velocity)->GetResource()
                    , name_velocity, FFX_RESOURCE_STATE_COMPUTE_READ);

                m_ffx_fsr2_dispatch_description.output = ffxGetResourceDX12(&m_ffx_fsr2_context, static_cast<RHI::DX12::RHI_Texture_DX12*>(tex_output)->GetResource()
                    , name_output, FFX_RESOURCE_STATE_UNORDERED_ACCESS);

                m_ffx_fsr2_dispatch_description.exposure = ffxGetResourceDX12(&m_ffx_fsr2_context, nullptr, L"FSR2_InputExposure", FFX_RESOURCE_STATE_GENERIC_READ);
                m_ffx_fsr2_dispatch_description.exposure = ffxGetResourceDX12(&m_ffx_fsr2_context, nullptr, L"FSR2_EmptyInputReactiveMap", FFX_RESOURCE_STATE_GENERIC_READ);
                m_ffx_fsr2_dispatch_description.exposure = ffxGetResourceDX12(&m_ffx_fsr2_context, nullptr, L"FSR2_EmptyTransparencyAndCompositionMap", FFX_RESOURCE_STATE_GENERIC_READ);
#endif
            }

            m_ffx_fsr2_dispatch_description.motionVectorScale.x     = -static_cast<float>(resolution_render_x);
            m_ffx_fsr2_dispatch_description.motionVectorScale.y     = -static_cast<float>(resolution_render_y);
            m_ffx_fsr2_dispatch_description.enableSharpening        = sharpness != 0.0f;
            m_ffx_fsr2_dispatch_description.sharpness               = sharpness;
            m_ffx_fsr2_dispatch_description.frameTimeDelta          = delta_time * 1000.0f;                     // Seconds to milliseconds.
            m_ffx_fsr2_dispatch_description.preExposure             = 1.0f;                                     // The exposure value if not using FFX_FSR2_ENABLE_AUTO_EXPOSURE.
            m_ffx_fsr2_dispatch_description.renderSize.width        = resolution_render_x;
            m_ffx_fsr2_dispatch_description.renderSize.height       = resolution_render_y;
            m_ffx_fsr2_dispatch_description.cameraNear              = camera_near_plane;
            m_ffx_fsr2_dispatch_description.cameraFar               = camera_far_plane;
            m_ffx_fsr2_dispatch_description.cameraFovAngleVertical  = camera_vertical_fov;
            m_ffx_fsr2_dispatch_description.reset                   = reset;                                    // A boolean value which when set to true, indicates the camera has moved discontinuously.

            ASSERT(ffxFsr2ContextDispatch(&m_ffx_fsr2_context, &m_ffx_fsr2_dispatch_description) == FFX_OK);

            cmd_list->SetImageLayout(tex_input, ImageLayout::ColourAttachment);
            cmd_list->SetImageLayout(tex_output, ImageLayout::ShaderReadOnly);
		}

        void RHI_FSR::CreateContext(u32 renderWidth, u32 renderHeight, u32 displayWidth, u32 displayHeight)
        {
            RenderContext* render_context = &RenderContext::Instance();
            render_context->GpuWaitForIdle();
            m_ffx_fsr2_context_description = {};

            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
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
                m_ffx_fsr2_context_description.maxRenderSize.width = std::max(1u, renderWidth);
                m_ffx_fsr2_context_description.maxRenderSize.height = std::max(1u, renderHeight);
                m_ffx_fsr2_context_description.displaySize.width = std::max(1u, displayWidth);
                m_ffx_fsr2_context_description.displaySize.height = std::max(1u, displayHeight);
                m_ffx_fsr2_context_description.flags = FFX_FSR2_ENABLE_AUTO_EXPOSURE;

                ffxFsr2ContextCreate(&m_ffx_fsr2_context, &m_ffx_fsr2_context_description);
#endif
            }
            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
            {
#ifdef IS_DX12_ENABLED
                RHI::DX12::RenderContext_DX12* renderContextDX12 = static_cast<RHI::DX12::RenderContext_DX12*>(render_context);
                const u64 scratchBufferSize = ffxFsr2GetScratchMemorySizeDX12();
                void* scratchBuffer = malloc(scratchBufferSize);
                FfxErrorCode errorCode = ffxFsr2GetInterfaceDX12(
                    &m_ffx_fsr2_context_description.callbacks
                    , renderContextDX12->GetDevice()
                    , scratchBuffer
                    , scratchBufferSize);
                ASSERT(errorCode == FFX_OK);

                m_ffx_fsr2_context_description.device = ffxGetDeviceDX12(renderContextDX12->GetDevice());
                m_ffx_fsr2_context_description.maxRenderSize.width = std::max(2u, std::min(renderWidth, displayWidth));
                m_ffx_fsr2_context_description.maxRenderSize.height = std::max(2u, std::min(renderHeight, displayHeight));
                m_ffx_fsr2_context_description.displaySize.width = std::max(2u, displayWidth);
                m_ffx_fsr2_context_description.displaySize.height = std::max(2u, displayHeight);
                //m_ffx_fsr2_context_description.flags = FFX_FSR2_ENABLE_AUTO_EXPOSURE;

                 FfxErrorCode createErrorCode = ffxFsr2ContextCreate(&m_ffx_fsr2_context, &m_ffx_fsr2_context_description);
                 ASSERT(createErrorCode == FFX_OK);
#endif
            }
        }
	}
}