#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUCommandBufferVulkan::GPUCommandBufferVulkan()
		: m_cmdBuffer(nullptr)
	{ }

	GPUCommandBufferVulkan::~GPUCommandBufferVulkan()
	{
		ReleaseGPU();
	}

	void GPUCommandBufferVulkan::Init(Graphics::GPUCommandBufferDesc const& desc)
	{
		ReleaseGPU();

		m_desc = desc;
		if (m_desc.CommandPool == nullptr)
		{
			m_desc.CommandPool = m_device->GetDefaultCommandPool();
		}
		GPUCommandPoolVulkan* cmdPool = static_cast<GPUCommandPoolVulkan*>(m_desc.CommandPool);
		cmdPool->AllocateCommandBuffer(this);
		m_memoryUsage = 1;
	}

	void GPUCommandBufferVulkan::BeginRecord()
	{
		ASSERT(	GetDesc().Usage == Graphics::GPUCommandBufferUsageFlags::SIMULATANEOUS_USE ? 
				m_state == Graphics::GPUCommandBufferState::IDLE || m_state == Graphics::GPUCommandBufferState::SUBMITTED :
				m_state == Graphics::GPUCommandBufferState::IDLE && 
				GetDesc().Usage == Graphics::GPUCommandBufferUsageFlags::SIMULATANEOUS_USE ? 
				"[GPUCommandBufferVulkan::BeginRecord] CommandBuffer 'SIMULATANEOUS_USE' is set. Command Buffer must be in the 'IDLE' or 'SUBMITTED' state." : 
				"[GPUCommandBufferVulkan::BeginRecord] CommandBuffer must be in the 'IDLE' state before recording again.");
	
		m_recordCommandCount = 0;
		m_state = Graphics::GPUCommandBufferState::RECORDING;
		VkCommandBufferBeginInfo info = vks::initializers::commandBufferBeginInfo();
		info.flags = ToVulkanCommandBufferUsageFlags(m_desc.Usage);
		ThrowIfFailed(vkBeginCommandBuffer(m_cmdBuffer, &info));
	}

	void GPUCommandBufferVulkan::EndRecord()
	{
		ASSERT(m_state == Graphics::GPUCommandBufferState::RECORDING && "[GPUCommandBufferVulkan::EndRecord] 'EndRecord' can only be called on a command buffer which is in the recording state.");
		vkEndCommandBuffer(m_cmdBuffer);
		m_state = Graphics::GPUCommandBufferState::IDLE;
	}

	void GPUCommandBufferVulkan::Reset()
	{
		ASSERT(m_state == Graphics::GPUCommandBufferState::IDLE && "[GPUCommandBufferVulkan::Reset] Command buffer must be in 'IDLE' state before being reset.");
		vkResetCommandBuffer(m_cmdBuffer, GetDesc().ReleaseResources == false ? 0 : VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	void GPUCommandBufferVulkan::Submit(GPUQueue queue)
	{
		ASSERT(m_state == Graphics::GPUCommandBufferState::IDLE && "[GPUCommandBufferVulkan::Submit] Command buffer must be in the 'IDLE' state to be submitted.");

		//TODO. Think about matching this with some form of queue system.
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_cmdBuffer;
		vkQueueSubmit(m_device->GetQueue(queue), 1, &submitInfo, VK_NULL_HANDLE);
		m_state = Graphics::GPUCommandBufferState::SUBMITTED;
	}

	void GPUCommandBufferVulkan::SubmitAndWait(GPUQueue queue)
	{
		ASSERT(m_state == Graphics::GPUCommandBufferState::IDLE && "[GPUCommandBufferVulkan::Submit] Command buffer must be in the 'IDLE' state to be submitted.");
		
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_cmdBuffer;

		auto fence = m_device->FenceManager.AllocateFence();
		ThrowIfFailed(vkQueueSubmit(m_device->GetQueue(queue), 1, &submitInfo, fence->GetHandle()));
		m_state = Graphics::GPUCommandBufferState::SUBMITTED;
		m_device->FenceManager.WaitAndReleaseFence(fence, U64_MAX);

		m_state = Graphics::GPUCommandBufferState::IDLE;
	}

	void GPUCommandBufferVulkan::OnReleaseGPU()
	{
		if (m_cmdBuffer)
		{
			/*ASSERT(m_cmdBuffer && "[GPUCommandBufferVulkan::OnReleaseGPU] Command buffer must be freed. This is done through the command pool it was allocated \
														or calling '::Delete' on the command buffer pointer.");*/
			m_desc.CommandPool->FreeCommandBuffer({ this });
			m_memoryUsage = 0;
		}
	}

	void GPUCommandBufferVulkan::BeginRenderpass(Graphics::GPURenderPass* renderpass)
	{
		++m_recordCommandCount;
	}

	void GPUCommandBufferVulkan::EndRenderpass(Graphics::GPURenderPass* renderpass)
	{
		++m_recordCommandCount;

	}

	void GPUCommandBufferVulkan::SetViewPort(Maths::Rect rect)
	{
		++m_recordCommandCount;

	}

	void GPUCommandBufferVulkan::SetScissor(Maths::Rect rect)
	{
		++m_recordCommandCount;

	}

	void Insight::GraphicsAPI::Vulkan::GPUCommandBufferVulkan::CopyBuffer(Graphics::GPUBuffer* srcBuffer, Graphics::GPUBuffer* dstBuffer, u32 regionCount, u64 srcOffset, u64 dstOffset, u64 size)
	{
		++m_recordCommandCount;
		GPUBufferVulkan* sourceBuffer = static_cast<GPUBufferVulkan*>(srcBuffer);
		GPUBufferVulkan* destinationBuffer = static_cast<GPUBufferVulkan*>(dstBuffer);
		VkBufferCopy copy = { };
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = size;
		vkCmdCopyBuffer(m_cmdBuffer, sourceBuffer->m_buffer, destinationBuffer->m_buffer, regionCount, &copy);
	}

	void GPUCommandBufferVulkan::BindDescriptorSets(PipelineBindPoint bindPoint, Graphics::GPUPipelineLayout* pipelineLayout, u32 firstSet, u32 descriptorSetCount, Graphics::GPUDescriptorSet* descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets)
	{
		++m_recordCommandCount;

	}

	void GPUCommandBufferVulkan::BindVertexBuffers(u32 firstBinding, u32 bindingCount, Graphics::GPUBuffer** buffers, u32* offsets)
	{
		++m_recordCommandCount;

		std::vector<VkBuffer> vkBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceSize> vkDeviceSizes = std::vector<VkDeviceSize>();
		vkBuffers.reserve(bindingCount);
		vkDeviceSizes.reserve(bindingCount);

		const u32 sizeOf = sizeof(GPUBufferVulkan);
		for (u32 i = 0; i < bindingCount; ++i)
		{
			GPUBufferVulkan* gpuBufferVulkan = static_cast<GPUBufferVulkan*>(buffers[i]);
			ASSERT((gpuBufferVulkan && gpuBufferVulkan->GetDesc().Flags & Graphics::GPUBufferFlags::VERTEX) && "[GPUCommandBufferVulkan::BindVertexBuffers] Buffer must be valid and of type vertex.");
			vkBuffers.push_back(gpuBufferVulkan->m_buffer);

			ASSERT(offsets && "[GPUCommandBufferVulkan::BindVertexBuffers] Offsets must be valid.");

			VkDeviceSize offset = static_cast<VkDeviceSize>(offsets[i]);
			vkDeviceSizes.push_back(offset);
		}
		vkCmdBindVertexBuffers(m_cmdBuffer, firstBinding, bindingCount, vkBuffers.data(), vkDeviceSizes.data());
	}

	void GPUCommandBufferVulkan::BindIndexBuffer(Graphics::GPUBuffer* buffer, u32 offset, Graphics::GPUCommandBufferIndexType indexType)
	{
		++m_recordCommandCount;
		ASSERT(buffer->GetDesc().Flags & Graphics::GPUBufferFlags::INDEX && "[GPUCommandBufferVulkan::BindVertexBuffers] Buffer must be of type index.");
		VkBuffer vkBuffer = static_cast<GPUBufferVulkan*>(buffer)->m_buffer;
		vkCmdBindIndexBuffer(m_cmdBuffer, vkBuffer, offset, (VkIndexType)indexType);
	}
	
	void GPUCommandBufferVulkan::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
	{
		++m_recordCommandCount;
		vkCmdDrawIndexed(m_cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	/// 
	// GPUCommandPoolVulkan
	/// 
	GPUCommandPoolVulkan::GPUCommandPoolVulkan()
		: m_cmdPool(nullptr)
	{ }

	GPUCommandPoolVulkan::~GPUCommandPoolVulkan()
	{
		ReleaseGPU();
	}

	void GPUCommandPoolVulkan::Init(Graphics::GPUCommandPoolDesc const& desc)
	{
		ReleaseGPU();

		m_desc = desc;
		VkCommandPoolCreateInfo info = vks::initializers::commandPoolCreateInfo();
		info.flags = ToVulkanCommandPoolUsageFlgas(m_desc.Flags);
		info.queueFamilyIndex = m_device->GetQueueFamilyIndex(m_desc.Queue);
		vkCreateCommandPool(m_device->Device, &info, nullptr, &m_cmdPool);
		m_memoryUsage = 1;
	}

	Graphics::GPUCommandBuffer* GPUCommandPoolVulkan::AllocateCommandBuffer(Graphics::GPUCommandBufferDesc& desc)
	{
		Graphics::GPUCommandBuffer* buffer = Graphics::GPUCommandBuffer::New();
		desc.CommandPool = this;
		buffer->Init(desc);
		return buffer;
	}

	std::vector<Graphics::GPUCommandBuffer*> GPUCommandPoolVulkan::AllocateCommandBuffers(Graphics::GPUCommandBufferDesc& desc, u32 count)
	{
		return std::vector<Graphics::GPUCommandBuffer*>();
	}

	void GPUCommandPoolVulkan::FreeCommandBuffer(std::vector<Graphics::GPUCommandBuffer*> buffers)
	{
		std::vector<VkCommandBuffer> vkCmdBuffers;
		vkCmdBuffers.reserve(buffers.size());
		for (auto const& cmdBuffer : buffers)
		{
			ASSERT((cmdBuffer && cmdBuffer->GetState() == Graphics::GPUCommandBufferState::IDLE) && "[GPUCommandPoolVulkan::FreeCommandBuffer] Command buffer must be valid and in the 'IDLE' state.");
			GPUCommandBufferVulkan* vkB = static_cast<GPUCommandBufferVulkan*>(cmdBuffer);
			vkCmdBuffers.push_back(vkB->m_cmdBuffer);

			vkB->m_cmdBuffer = nullptr;
			m_buffers.erase(std::find(m_buffers.begin(), m_buffers.end(), cmdBuffer));
		}
		vkFreeCommandBuffers(m_device->Device, m_cmdPool, static_cast<u32>(vkCmdBuffers.size()), vkCmdBuffers.data());
	}

	void GPUCommandPoolVulkan::OnReleaseGPU()
	{
		ASSERT(m_buffers.size() == 0 && "[GPUCommandPoolVulkan::OnReleaseGPU] Command pool has allocated buffer. All buffers must be freed before command pool is released.");
		m_memoryUsage = 0;
		vkDestroyCommandPool(m_device->Device, m_cmdPool, nullptr);
		m_cmdPool = nullptr;
	}

	void GPUCommandPoolVulkan::AllocateCommandBuffer(GPUCommandBufferVulkan* buffer)
	{
		ASSERT(!buffer->m_cmdBuffer && "[GPUCommandPoolVulkan::AllocateCommandBuffer] Command buffer handle must be null before being allocated.");
		VkCommandBufferAllocateInfo info = vks::initializers::commandBufferAllocateInfo(m_cmdPool, (VkCommandBufferLevel)buffer->GetDesc().Level, 1);
		ThrowIfFailed(vkAllocateCommandBuffers(m_device->Device, &info, &buffer->m_cmdBuffer));
		m_buffers.push_back(buffer);
	}

	void GPUCommandPoolVulkan::FreeCommandBuffer(GPUCommandBufferVulkan* buffer)
	{
		ASSERT(buffer->m_cmdBuffer && "[GPUCommandPoolVulkan::AllocateCommandBuffer] Command buffer handle must be valid before being freed.");
		vkFreeCommandBuffers(m_device->Device, m_cmdPool, 1, &buffer->m_cmdBuffer);	
		buffer->m_cmdBuffer = nullptr;
		m_buffers.erase(std::find(m_buffers.begin(), m_buffers.end(), buffer));
	}
}