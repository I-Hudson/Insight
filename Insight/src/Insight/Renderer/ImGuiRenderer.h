#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Event/ApplicationEvent.h"
#include "examples/imgui_impl_glfw.h"

#ifdef IS_VULKAN
#include "examples/imgui_impl_vulkan.h"
#endif // IS_VULKAN


namespace Insight
{
	class Renderer;

	namespace Render
	{
		class VulkanFramebuffer;
		class VulkanRenderer;
		class Shader;
		class VulkanMaterial;
		class CommandPool;
		class CommandBuffer;
	}
}

namespace Platform
{
	class VulkanRenderer;
	class CommandPool;
	class CommandBuffer;
	class VulkanFramebuffer;
}

class ImGuiRenderer : public Insight::TSingleton<ImGuiRenderer>
{
public:
	ImGuiRenderer(Insight::Renderer* renderer);
	~ImGuiRenderer();

	void NewFrame();
	void EndFrame();
	void Render(Platform::CommandBuffer* commandBuffer);

private:
	void WindowResize(const Insight::Event& event);

#ifdef IS_VULKAN
	void CreateDescPool();

	VkDescriptorPool m_descPool;
	Platform::VulkanRenderer* m_vulkanRenderer;
	Platform::CommandPool* m_commandPool;
	Platform::CommandBuffer* m_commandBuffer;
	Platform::VulkanFramebuffer* m_framebuffer;

	VkImage m_fontImage;
	VkImageView m_fontImageView;
	VkSampler m_fontSampler;
	VkDeviceMemory m_fontMemory;

	VkDeviceSize m_bufferMemoryAlignment = 256;
	VkDeviceMemory m_uploadBufferMemory = VK_NULL_HANDLE;
	VkBuffer m_uploadBuffer = VK_NULL_HANDLE;
#endif
};