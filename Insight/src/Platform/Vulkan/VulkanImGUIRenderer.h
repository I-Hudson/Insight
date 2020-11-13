#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/ImGuiRenderer.h"
#include "VulkanHeader.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include <imgui.h>
#include <glm/glm.hpp>

namespace vks
{
	class VulkanImGUIRenderer : public Insight::ImGuiRenderer
	{
	public:
		VulkanImGUIRenderer();
		virtual ~VulkanImGUIRenderer() override;

		virtual void NewFrame() override;
		virtual void EndFrame() override;
		void Render(VkCommandBuffer commandBuffer);

		// UI params are set via push constants
		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

	private:
		virtual void Init(Insight::Renderer* renderer) override;
		void InitResources();

		void UpdateBuffers();

		void WindowResize(Insight::Event const& event);

	private:
		bool m_newFrameInit = false;
		VulkanRenderer* m_renderer;

		// Vulkan resources for rendering the UI
		VkSampler             sampler;
		vks::VulkanBuffer     vertexBuffer;
		vks::VulkanBuffer     indexBuffer;
		int32_t               vertexCount = 0;
		int32_t               indexCount = 0;
		VkDeviceMemory        fontMemory = VK_NULL_HANDLE;
		VkImage               fontImage = VK_NULL_HANDLE;
		VkImageView           fontView = VK_NULL_HANDLE;
		VkPipelineCache       pipelineCache;
		VkPipelineLayout      pipelineLayout;
		VkPipeline            pipeline;
		VkDescriptorPool      descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet       descriptorSet;
		vks::VulkanDevice*	  device;
	};
}