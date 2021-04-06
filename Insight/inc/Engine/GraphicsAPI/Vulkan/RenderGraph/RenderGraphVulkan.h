#pragma once

#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"

class RenderGraphVulkan : public RenderGraph
{
public:
	virtual void EnqueueRenderPass() override;

private:
	struct PassSubmissionState
	{
		std::vector<VkBufferMemoryBarrier> BufferBarriers;
		std::vector<VkImageMemoryBarrier> ImageBarriers;
	
		// Immediate buffer barriers are useless because they don't need any layout transition,
		// and the API guarantees that submitting a batch makes memory visible to GPU resources.
		// Immediate image barriers are purely for doing layout transitions without waiting (srcStage = TOP_OF_PIPE).
		std::vector<VkImageMemoryBarrier> ImmediateImageBarriers;

		// Barriers which are used when waiting for a semaphore, and then doing a transition.
		// We need to use pipeline barriers here so we can have srcStage = dstStage,
		// and hand over while not breaking the pipeline.
		std::vector<VkImageMemoryBarrier> SemaphoreHandoverBarriers;
		std::vector<VkEvent> Events;

		std::vector<VkSubpassContents> SubpassContents;

		VkPipelineStageFlags DstStages = 0;
		VkPipelineStageFlags ImmediateDstStages = 0;
		VkPipelineStageFlags SrcStages = 0;
		VkPipelineStageFlags HandoverStages = 0;

		std::vector<SemaphoreVulkan> WaitSemaphores;
		std::vector<VkPipelineStageFlags> WaitSemaphoreStages;

		PipelineEventVulkan SignalEvent;
		VkPipelineStageFlags EventSignalStages = 0;

		SemaphoreVulkan ProxySemaphores[2];
		bool NeedSubmissionSemaphore = false;

		//Vulkan::CommandBufferHandle Cmd;

		//Vulkan::CommandBuffer::Type QueueType = Vulkan::CommandBuffer::Type::Count;
		bool Graphics = false;
		bool Active = false;

		//TaskGroupHandle rendering_dependency;

		void AddUniqueEvent(VkEvent event);
		void EmitPrePassBarriers();
		void EmitPostPassBarriers();
		void Submit();
	};

	std::vector<PassSubmissionState> m_passSubmissionStates;
};

