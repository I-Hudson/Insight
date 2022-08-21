#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RenderContext.h"

#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

//TODO: Remove this whole class.
/*
	We should be using a simple struct like
	struct RenderDrawData
	{
		Pso PipelineStateObject

		RHI_Buffer* VertexBuffer
		RHI_Buffer* IndexBuffer

		UniformGroup Uniforms

		int VertexIndex
		int IndexIndex
	};

	to keep track of the draw data we want to use.
*/

namespace Insight
{
	namespace Graphics
	{
		/// <summary>
		/// RHI_CommandList
		/// </summary>
		/// <returns></returns>
		RHI_CommandList* RHI_CommandList::New()
		{
#if defined(IS_VULKAN_ENABLED)
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_CommandList_Vulkan); }
#endif
#if defined(IS_DX12_ENABLED)
			if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_CommandList_DX12); }
#endif
			return nullptr;
		}

		void RHI_CommandList::Reset()
		{
			m_pso = {};
			m_activePSO = {};
			m_drawData = {};
		}

		bool RHI_CommandList::CanDraw()
		{
			if (m_pso.GetHash() != m_activePSO.GetHash())
			{
				m_activePSO = m_pso;
				RHI_DescriptorLayout* layout = m_context->GetDescriptorLayoutManager().GetLayout(m_activePSO.Shader->GetDescriptors()
				/*cmdList.GetDescriptorBuffer().GetDescriptorsSignature()*/);
				BindPipeline(m_activePSO, layout);
			}
			return BindDescriptorSets();
		}

		bool RHI_CommandList::BindDescriptorSets()
		{
			return false;
		}


		/// <summary>
		/// RHI_CommandListAllocator
		/// </summary>
		/// <returns></returns>
		RHI_CommandListAllocator* RHI_CommandListAllocator::New()
		{
#if defined(IS_VULKAN_ENABLED)
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_CommandListAllocator_Vulkan); }
#endif
#if defined(IS_DX12_ENABLED)
			if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_CommandListAllocator_DX12); }
#endif
			return nullptr;
		}

		void RHI_CommandListAllocator::ReturnCommandList(RHI_CommandList* cmdList)
		{
			//if (m_allocLists.find(cmdList) == m_allocLists.end())
			//{
			//	IS_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is not in the allocated list. Command lists should be obtained by 'GetCommandList'.");
			//}
			//m_allocLists.erase(cmdList);
			//
			//if (m_freeLists.find(cmdList) != m_freeLists.end())
			//{
			//	IS_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is in the free list. Command should not be returned more than once.");
			//	return;
			//}
			//m_freeLists.insert(cmdList);
		}

		/// <summary>
		/// CommandListManager
		/// </summary>
		CommandListManager::CommandListManager()
		{
		}

		CommandListManager::~CommandListManager()
		{
		}

		void CommandListManager::Create(RenderContext* context)
		{
			m_context = context;

			if (m_allocator)
			{
				IS_CORE_WARN("[CommandListManager::Init] CommandListManager already has 'Init' called.");
				return;
			}

			m_allocator = RHI_CommandListAllocator::New();
			m_allocator->Create(m_context);
		}

		void CommandListManager::Update()
		{
			m_allocator->Reset();
		}

		void CommandListManager::Destroy()
		{
			if (m_allocator)
			{
				m_allocator->Release();
				DeleteTracked(m_allocator);
			}
		}

		RHI_CommandList* CommandListManager::GetCommandList()
		{
			assert(m_allocator);
			return m_allocator->GetCommandList();
		}

		RHI_CommandList* CommandListManager::GetSingleUseCommandList()
		{
			assert(m_allocator);
			return m_allocator->GetSingleSubmitCommandList();
		}

		void CommandListManager::ReturnCommandList(RHI_CommandList* cmdList)
		{
			assert(m_allocator);
			m_allocator->ReturnCommandList(cmdList);
		}

		void CommandListManager::ReturnSingleUseCommandList(RHI_CommandList* cmdList)
		{
			assert(m_allocator);
			m_allocator->ReturnSingleSubmitCommandList(cmdList);
		}

		void RHI_CommandList::SetUniform(int set, int binding, void* data, u32 size)
		{
			m_descriptorAllocator->SetUniform(set, binding, data, size);
		}

		void RHI_CommandList::SetTexture(int set, int binding, RHI_Texture* texture)
		{
		}
}
}