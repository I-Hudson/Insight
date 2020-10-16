#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan/Vulkan.h"
#include "Insight/Renderer/Buffer.h"

namespace Platform
{
	class VulkanRenderer;

	class IS_API VulkanVertexBuffer : public Insight::Render::VertexBuffer
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

	class VulkanIndexBuffer : public Insight::Render::IndexBuffer
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
#endif