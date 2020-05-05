#include "ispch.h"
#include "VulkanBuffers.h"

#include "Insight/Memory/MemoryManager.h"

#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Renderer/VulkanRenderer.h"
#include "Insight/Renderer/Lowlevel/Device.h"

#include "Insight/Renderer/Lowlevel/CommandPool.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"

namespace Insight
{
	namespace Render
	{
		VulkanRenderer* VulkanVertexBuffer::s_Renderer;
		VulkanRenderer* VulkanIndexBuffer::s_Renderer;

		void CreateBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
		{
			VkBufferCreateInfo createInfo = VulkanInits::BufferInfo();
			createInfo.size = size;
			createInfo.usage = usage;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			ThrowIfFailed(vkCreateBuffer(device->GetDevice(), &createInfo, nullptr, &buffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device->GetDevice(), buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = VulkanInits::MemoryAllocInfo();
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = device->GetMemoryType(memRequirements.memoryTypeBits, properties);

			ThrowIfFailed(vkAllocateMemory(device->GetDevice(), &allocInfo, nullptr, &bufferMemory));
			vkBindBufferMemory(device->GetDevice(), buffer, bufferMemory, 0);
		}

		void CopyBuffer(Device* device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
		{
			CommandPool* commandPool = Memory::MemoryManager::NewOnFreeList<CommandPool>(device);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool->GetCommandPool();
			allocInfo.commandBufferCount = 1;

			CommandBuffer* commandBuffer = commandPool->AllocCommandBuffer();
			vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer->GetBuffer());

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer->GetBuffer(), &beginInfo);

			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer->GetBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

			vkEndCommandBuffer(commandBuffer->GetBuffer());

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer->GetBuffer();

			vkQueueSubmit(device->GetQueue(QueueFamilyType::Graphics).GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(device->GetQueue(QueueFamilyType::Graphics).GetQueue());

			commandPool->FreeCommandBuffers();
			Memory::MemoryManager::DeleteOnFreeList(commandPool);
		}

		VulkanVertexBuffer::VulkanVertexBuffer(const std::vector<Vertex>& vertices)
			: VertexBuffer()
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
			CreateBuffer(s_Renderer->GetDeviceWrapper(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(s_Renderer->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(s_Renderer->GetDevice(), stagingBufferMemory);

			CreateBuffer(s_Renderer->GetDeviceWrapper(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMem);

			CopyBuffer(s_Renderer->GetDeviceWrapper(), stagingBuffer, m_buffer, bufferSize);

			vkDestroyBuffer(s_Renderer->GetDevice(), stagingBuffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), stagingBufferMemory, nullptr);

			IS_INFO("VERTEX BUFFER CREATED!!!");
		}

		VulkanVertexBuffer::~VulkanVertexBuffer()
		{
			vkDestroyBuffer(s_Renderer->GetDevice(), m_buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_bufferMem, nullptr);
		}

		VulkanIndexBuffer::VulkanIndexBuffer(const std::vector<unsigned int>& indices)
			: IndexBuffer()
		{
			VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(s_Renderer->GetDeviceWrapper(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(s_Renderer->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, indices.data(), (size_t)bufferSize);
			vkUnmapMemory(s_Renderer->GetDevice(), stagingBufferMemory);

			CreateBuffer(s_Renderer->GetDeviceWrapper(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMem);

			CopyBuffer(s_Renderer->GetDeviceWrapper(), stagingBuffer, m_buffer, bufferSize);

			vkDestroyBuffer(s_Renderer->GetDevice(), stagingBuffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), stagingBufferMemory, nullptr);
		}

		VulkanIndexBuffer::~VulkanIndexBuffer()
		{
			vkDestroyBuffer(s_Renderer->GetDevice(), m_buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_bufferMem, nullptr);
		}
	}
}