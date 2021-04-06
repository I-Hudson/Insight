#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"

#include "Engine/Graphics/GPUBuffer.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUCommandBufferVulkan::GPUCommandBufferVulkan()
		: m_cmdBuffer(nullptr)
	{
	}

	GPUCommandBufferVulkan::~GPUCommandBufferVulkan()
	{
	}

	void GPUCommandBufferVulkan::Init(Graphics::GPUCommandBufferDesc const& desc)
	{
	}

	void GPUCommandBufferVulkan::BeginRecord()
	{
		VkCommandBufferBeginInfo info = vks::initializers::commandBufferBeginInfo();
		info.flags = ToVulkanCommandBufferUsageFlags(m_desc.Usage);
		ThrowIfFailed(vkBeginCommandBuffer(m_cmdBuffer, &info));
	}

	void GPUCommandBufferVulkan::EndRecord()
	{
		vkEndCommandBuffer(m_cmdBuffer);
	}

	void GPUCommandBufferVulkan::Submit()
	{
	}

	void GPUCommandBufferVulkan::Clear()
	{
	}

	void GPUCommandBufferVulkan::OnReleaseGPU()
	{
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

	void GPUCommandBufferVulkan::BindDescriptorSets(PipelineBindPoint bindPoint, Graphics::GPUPipelineLayout* pipelineLayout, u32 firstSet, u32 descriptorSetCount, Graphics::GPUDescriptorSet* descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets)
	{
		++m_recordCommandCount;

	}

	void GPUCommandBufferVulkan::BindVertexBuffers(u32 firstBinding, u32 bindingCount, GPUBuffer* buffers, u32* offsets)
	{
		++m_recordCommandCount;

		std::vector<VkBuffer> vkBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceSize> vkDeviceSizes = std::vector<VkDeviceSize>();
		vkBuffers.reserve(bindingCount);
		vkDeviceSizes.reserve(bindingCount);

		for (u32 i = 0; i < bindingCount; ++i)
		{
			//TODO: Finish this.
			// Make the assumption that buffers and offsets is a pointer to the first element in an array/vector.
			ASSERT((buffers + (sizeof(GPUBuffer) * i))->GetDescription().Flags == GPUBufferFlags::VertexBuffer && "[GPUCommandBufferVulkan::BindVertexBuffers] Buffer must be of type vertex.");
			//GPUBufferVulkan* gpuVkBuffer = static_cast<GPUBufferVulkan*>(buffers[i]->GetBuffer());
			//vkBuffers.push_back()
			vkDeviceSizes.push_back(*(offsets + (sizeof(u32) * i)));
		}
		vkCmdBindVertexBuffers(m_cmdBuffer, firstBinding, bindingCount, vkBuffers.data(), vkDeviceSizes.data());
	}

	void GPUCommandBufferVulkan::BindIndexBuffers(GPUBuffer* buffer, u32 offset, Graphics::GPUCommandBufferIndexType indexType)
	{
		++m_recordCommandCount;
		ASSERT(buffer->GetDescription().Flags == GPUBufferFlags::IndexBuffer && "[GPUCommandBufferVulkan::BindVertexBuffers] Buffer must be of type index.");
		VkBuffer vkBuffer = nullptr; //static_cast<GPUBufferVulkan*>(buffer->GetBuffer());
		vkCmdBindIndexBuffer(m_cmdBuffer, vkBuffer, offset, (VkIndexType)indexType);
	}

	void GPUCommandBufferVulkan::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
	{
		++m_recordCommandCount;
		vkCmdDrawIndexed(m_cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
}