#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUAdapterVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDynamicBufferVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUImageVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSyncVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"

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

	void GPUCommandBufferVulkan::Submit(GPUQueue queue, Graphics::GPUFence* fence)
	{
		ASSERT(m_state == Graphics::GPUCommandBufferState::IDLE && "[GPUCommandBufferVulkan::Submit] Command buffer must be in the 'IDLE' state to be submitted.");

		GPUFenceVulkan* fenceVulkan = nullptr;
		if (fence != nullptr)
		{
			fenceVulkan = static_cast<GPUFenceVulkan*>(fence);
		}

		//TODO. Think about matching this with some form of queue system.
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_cmdBuffer;
		vkQueueSubmit(m_device->GetQueue(queue), 1, &submitInfo, fenceVulkan ? *fenceVulkan->GetHandleVulkan() : nullptr);
		m_state = Graphics::GPUCommandBufferState::SUBMITTED;
	}

	void GPUCommandBufferVulkan::SubmitAndWait(GPUQueue queue)
	{
		ASSERT(m_state == Graphics::GPUCommandBufferState::IDLE && "[GPUCommandBufferVulkan::Submit] Command buffer must be in the 'IDLE' state to be submitted.");
		
		VkSubmitInfo submitInfo = vks::initializers::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_cmdBuffer;

		GPUFenceVulkan* fence = static_cast<GPUFenceVulkan*>(Graphics::GPUFence::New());
		fence->Init(Graphics::GPUFenceDesc());
		ThrowIfFailed(vkQueueSubmit(m_device->GetQueue(queue), 1, &submitInfo, *fence->GetHandleVulkan()));
		m_state = Graphics::GPUCommandBufferState::SUBMITTED;
		fence->Wait();
		::Delete(fence);

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

	void GPUCommandBufferVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Command_Buffer, (u64)m_cmdBuffer);
		}
	}

	void GPUCommandBufferVulkan::BeginRenderpass(Graphics::GPURenderGraphPass* renderpass)
	{
		++m_recordCommandCount;
		GPURenderGraphPassVulkan* renderPass = static_cast<GPURenderGraphPassVulkan*>(renderpass);

		std::vector<VkClearValue> clearColors;
		for (size_t i = 0; i < renderPass->m_colorAttachmentsCount; ++i)
		{
			VkClearValue v;
			v.color.float32[0] = renderPass->GetRenderPass().GetClearColor().x;
			v.color.float32[1] = renderPass->GetRenderPass().GetClearColor().y;
			v.color.float32[2] = renderPass->GetRenderPass().GetClearColor().z;
			v.color.float32[3] = renderPass->GetRenderPass().GetClearColor().w;
			clearColors.push_back(v);
		}
		if (renderPass->GetRenderPass().IsDepthSencilOuputValid())
		{
			VkClearValue v;
			v.depthStencil.depth = (float)renderPass->GetRenderPass().GetClearDepthStencil().x;
			v.depthStencil.stencil = (u32)renderPass->GetRenderPass().GetClearDepthStencil().y;
			clearColors.push_back(v);
		}

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = renderPass->m_renderPass;
		renderPassBeginInfo.framebuffer = renderPass->m_frameBuffer;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = (u32)renderPass->GetRenderPass().GetWindowRect().GetWidth();
		renderPassBeginInfo.renderArea.extent.height = (u32)renderPass->GetRenderPass().GetWindowRect().GetHeight();
		renderPassBeginInfo.clearValueCount = static_cast<U32>(clearColors.size());
		renderPassBeginInfo.pClearValues = clearColors.data();
		vkCmdBeginRenderPass(m_cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void GPUCommandBufferVulkan::EndRenderpass(Graphics::GPURenderGraphPass* renderpass)
	{
		++m_recordCommandCount;
		vkCmdEndRenderPass(m_cmdBuffer);
	}

	void GPUCommandBufferVulkan::SetViewPort(Maths::Rect rect)
	{
		++m_recordCommandCount;
		VkViewport viewPorts[] = { ToVulkanViewPort(rect) };
		vkCmdSetViewport(m_cmdBuffer, 0, 1, &viewPorts[0]);
	}

	void GPUCommandBufferVulkan::SetScissor(Maths::Rect rect)
	{
		++m_recordCommandCount;
		vkCmdSetScissor(m_cmdBuffer, 0, 1, nullptr);
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

	void GPUCommandBufferVulkan::CopyBufferToDynamic(Graphics::GPUBuffer* srcBuffer, Graphics::GPUDynamicBuffer* dstBuffer, u32 regionCount, u64 srcOffset, u64 dstOffset, u64 size)
	{
		++m_recordCommandCount;
		GPUBufferVulkan* sourceBuffer = static_cast<GPUBufferVulkan*>(srcBuffer);
		GPUDynamicBufferVulkan* destinationBuffer = static_cast<GPUDynamicBufferVulkan*>(dstBuffer);
		VkBufferCopy copy = { };
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = size;
		vkCmdCopyBuffer(m_cmdBuffer, sourceBuffer->m_buffer, destinationBuffer->m_buffer, regionCount, &copy);
	}

	void GPUCommandBufferVulkan::CopyBufferToImage(Graphics::GPUBuffer* srcBuffer, Graphics::GPUImage* dstImage, Graphics::GPUImageDesc const* imageDesc)
	{
		++m_recordCommandCount;
		GPUBufferVulkan* sourceBuffer = static_cast<GPUBufferVulkan*>(srcBuffer);
		GPUImageVulkan* destinationImage = static_cast<GPUImageVulkan*>(dstImage);

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < imageDesc->Levels; ++i)
		{
			// Setup a buffer image copy structure for the current mip level
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = imageDesc->Width >> i;
			bufferCopyRegion.imageExtent.height = imageDesc->Height >> i;
			bufferCopyRegion.imageExtent.depth = 1;
			//bufferCopyRegion.bufferOffset = GetMipMapOffset(imageDesc.Width, imageDesc.Height, 4, i);
			bufferCopyRegions.push_back(bufferCopyRegion);
		}

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		VkImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = imageDesc->Levels;
		// The 2D texture only has one layer
		subresourceRange.layerCount = imageDesc->Layers;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
		imageMemoryBarrier.image = destinationImage->m_vImage;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		vkCmdPipelineBarrier(m_cmdBuffer,
							 VK_PIPELINE_STAGE_HOST_BIT,
							 VK_PIPELINE_STAGE_TRANSFER_BIT,
							 0,
							 0, nullptr,
							 0, nullptr,
							 1, &imageMemoryBarrier);

		vkCmdCopyBufferToImage(m_cmdBuffer,
							   sourceBuffer->m_buffer,
							   destinationImage->m_vImage,
							   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   static_cast<u32>(bufferCopyRegions.size()),
							   bufferCopyRegions.data());

		// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
		vkCmdPipelineBarrier(
			m_cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	void GPUCommandBufferVulkan::BlipImageToSwapchain(Graphics::GPUImage* srcImage, Graphics::GPUImage* dstImage)
	{
		++m_recordCommandCount;

		GPUImageVulkan* sourceImage = static_cast<GPUImageVulkan*>(srcImage);
		GPUImageVulkan* destinationImage = static_cast<GPUImageVulkan*>(dstImage);

		VkImageSubresourceRange srcSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		VkImageMemoryBarrier srcImageMemoryBarrier = vks::initializers::imageMemoryBarrier();
		srcImageMemoryBarrier.image = sourceImage->m_vImage;
		srcImageMemoryBarrier.subresourceRange = srcSubresourceRange;
		srcImageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		srcImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		srcImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		srcImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		vkCmdPipelineBarrier(
			m_cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &srcImageMemoryBarrier);

		VkImageSubresourceRange dstSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		VkImageMemoryBarrier dstImageMemoryBarrier = vks::initializers::imageMemoryBarrier();
		dstImageMemoryBarrier.image = destinationImage->m_vImage;
		dstImageMemoryBarrier.subresourceRange = dstSubresourceRange;
		dstImageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		dstImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dstImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		dstImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdPipelineBarrier(
			m_cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &dstImageMemoryBarrier);

		// Check blit support for source and destination
		VkFormatProperties formatProps;
		GPUAdapterVulkan* adapter = static_cast<GPUAdapterVulkan*>(m_device->GetAdapter());
		bool supportsBlit = true;

		// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
		vkGetPhysicalDeviceFormatProperties(adapter->Gpu, ToVulkanFormat(dstImage->GetDesc().Format), &formatProps);
		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
		{
			supportsBlit = false;
		}

		if (supportsBlit)
		{
			// Define the region to blit (we will blit the whole swapchain image)
			VkOffset3D blitSize;
			blitSize.x = dstImage->GetDesc().Width;
			blitSize.y = dstImage->GetDesc().Height;
			blitSize.z = 1;
			VkImageBlit imageBlitRegion{};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1] = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1] = blitSize;

			// Issue the blit command
			vkCmdBlitImage(
				m_cmdBuffer,
				sourceImage->m_vImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				destinationImage->m_vImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
		}
		else
		{
			// Otherwise use image copy (requires us to manually flip components)
			VkImageCopy imageCopyRegion{};
			imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.srcSubresource.layerCount = 1;
			imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.dstSubresource.layerCount = 1;
			imageCopyRegion.extent.width = dstImage->GetDesc().Width;
			imageCopyRegion.extent.height = dstImage->GetDesc().Height;
			imageCopyRegion.extent.depth = 1;

			// Issue the copy command
			vkCmdCopyImage(
				m_cmdBuffer,
				sourceImage->m_vImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				destinationImage->m_vImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageCopyRegion);
		}

		srcSubresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		srcImageMemoryBarrier = vks::initializers::imageMemoryBarrier();
		srcImageMemoryBarrier.image = sourceImage->m_vImage;
		srcImageMemoryBarrier.subresourceRange = srcSubresourceRange;
		srcImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcImageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		srcImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		srcImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		vkCmdPipelineBarrier(
			m_cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &srcImageMemoryBarrier);

		dstSubresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		dstImageMemoryBarrier = vks::initializers::imageMemoryBarrier();
		dstImageMemoryBarrier.image = destinationImage->m_vImage;
		dstImageMemoryBarrier.subresourceRange = dstSubresourceRange;
		dstImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dstImageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dstImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		dstImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		vkCmdPipelineBarrier(
			m_cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &dstImageMemoryBarrier);


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

	void GPUCommandPoolVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Command_Pool, (u64)m_cmdPool);
		}
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

	void GPUCommandPoolVulkan::Reset()
	{
		ThrowIfFailed(vkResetCommandPool(m_device->Device, m_cmdPool, 0));
		for (auto* cmdBuffer : m_buffers)
		{
			static_cast<GPUCommandBufferVulkan*>(cmdBuffer)->m_state = Graphics::GPUCommandBufferState::IDLE;
		}
	}
}