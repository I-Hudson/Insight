#pragma once

#include "Graphics/CommandList.h"
#include <vulkan/vulkan.hpp>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;

			class CommandList_Vulkan
			{
			public:
				CommandList_Vulkan(RenderContext_Vulkan* context, vk::CommandBuffer cmdBuffer);

				void Record(CommandList& cmdList);
				void Reset();
				vk::CommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

				bool operator==(const CommandList_Vulkan& other) const { return m_commandBuffer == other.m_commandBuffer; }

			private:
				bool CanDraw();

			private:
				vk::CommandBuffer m_commandBuffer;
				RenderContext_Vulkan* m_context;

				bool m_activeRenderpass = false;
				PipelineStateObject m_pso;
				PipelineStateObject m_activePSO;
				vk::Framebuffer m_framebuffer;
			};

			class CommandPool_Vulkan
			{
			public:

				void Init(RenderContext_Vulkan* context);
				void Update();
				void Destroy();

				CommandList_Vulkan& GetCommandList();
				void ReturnCommandList(CommandList_Vulkan& cmdList);

			private:
				RenderContext_Vulkan* m_context{ nullptr };

				std::vector<CommandList_Vulkan> m_allocLists;
				std::vector<CommandList_Vulkan> m_freeLists;

				vk::CommandPool m_pool{ nullptr };
			};
		}
	}
}