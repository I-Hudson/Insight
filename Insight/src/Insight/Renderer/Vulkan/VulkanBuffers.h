#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

#include "Insight/Renderer/Buffer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Insight
{
	namespace Render
	{
		class VulkanRenderer;

		class IS_API VulkanVertexBuffer : public VertexBuffer
		{
		public:
			VulkanVertexBuffer(const std::vector<Vertex>& vertices);
			virtual ~VulkanVertexBuffer() override;

			VkBuffer GetBuffer() const { return m_buffer; }

		private:
			VkBuffer m_buffer;
			VkDeviceMemory m_bufferMem;

			static VulkanRenderer* s_Renderer;
			friend VulkanRenderer;
		};

		class VulkanIndexBuffer : public IndexBuffer
		{
		public:
			VulkanIndexBuffer(const std::vector<unsigned int>& indices);
			virtual ~VulkanIndexBuffer() override;

			VkBuffer GetBuffer() const { return m_buffer; }

		private:
			VkBuffer m_buffer;
			VkDeviceMemory m_bufferMem;

			static VulkanRenderer* s_Renderer;
			friend VulkanRenderer;
		};
	}
}
#endif