#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Event/ApplicationEvent.h"
#include "examples/imgui_impl_glfw.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/Vulkan.h"
#endif

namespace Insight
{
	class Renderer;

	namespace Render
	{
		class Shader;
		class Framebuffer;
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

	Insight::Render::Framebuffer* m_framebuffer;

#ifdef IS_VULKAN
	void CreateDescPool();

	VkDescriptorPool m_descPool;
	Platform::VulkanRenderer* m_vulkanRenderer;
	Platform::CommandPool* m_commandPool;
	Platform::CommandBuffer* m_commandBuffer;

	VkImage m_fontImage;
	VkImageView m_fontImageView;
	VkSampler m_fontSampler;
	VkDeviceMemory m_fontMemory;

	VkDeviceSize m_bufferMemoryAlignment = 256;
	VkDeviceMemory m_uploadBufferMemory = 0;
	VkBuffer m_uploadBuffer = 0;
#endif
};