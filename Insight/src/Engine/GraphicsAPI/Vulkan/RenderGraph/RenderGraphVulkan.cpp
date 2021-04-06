#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"

void RenderGraphVulkan::EnqueueRenderPass()
{
	auto& device = *(GPUDeviceVulkan*)GPUDeviceVulkan::Instance();

	auto* event = device.PipelineEventManger.AllocateEvent(false);
	device.PipelineEventManger.IsEventSignaled(event);
	device.PipelineEventManger.ReleaseEvent(event);
}
