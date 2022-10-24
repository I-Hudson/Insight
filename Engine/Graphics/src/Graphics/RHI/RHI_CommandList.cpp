#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RenderContext.h"

#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		//// <summary>
		//// RHI_CommandList
		//// </summary>
		//// <returns></returns>
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
			m_bound_vertex_buffer = nullptr;
			m_bound_index_buffer = nullptr;
			m_discard = false;
		}

		void RHI_CommandList::SetImageLayout(RHI_Texture* texture, ImageLayout layout)
		{
			RHI_TextureInfo create_info = texture->GetInfo();
			bool transition_requried = false;

			for (u32 i = 0; i < create_info.Mip_Count; ++i)
			{
				if (texture->GetLayout(i) != layout)
				{
					transition_requried = true;
				}
			}

			if (transition_requried)
			{
				SetImageLayoutTransition(texture, layout);

				for (size_t i = 0; i < create_info.Mip_Count; ++i)
				{
					texture->m_infos[i].Layout = layout;
				}
			}
		}

		void RHI_CommandList::SetUniform(u32 set, u32 binding, const void* data, u32 size)
		{
			m_descriptorAllocator->SetUniform(set, binding, data, size);
		}

		void RHI_CommandList::SetTexture(u32 set, u32 binding, const RHI_Texture* texture)
		{
			m_descriptorAllocator->SetTexture(set, binding, texture, nullptr);
			m_context->GetResourceRenderTracker().TrackResource(texture);
		}

		void RHI_CommandList::SetTexture(u32 set, u32 binding, const RHI_Texture* texture, const RHI_Sampler* sampler)
		{
			m_descriptorAllocator->SetTexture(set, binding, texture, sampler);
			m_context->GetResourceRenderTracker().TrackResource(texture);
		}

		void RHI_CommandList::SetSampler(u32 set, u32 binding, const RHI_Sampler* sampler)
		{
			m_descriptorAllocator->SetSampler(set, binding, sampler);
		}

		RHI_BufferView RHI_CommandList::UploadUniform(const void* data, u32 size)
		{
			return m_descriptorAllocator->UploadUniform(data, size);
		}

		void RHI_CommandList::SetUniform(u32 set, u32 binding, RHI_BufferView buffer)
		{
			m_descriptorAllocator->SetUniform(set, binding, buffer);
		}

		void RHI_CommandList::BeginTimeBlock(const char* block_name)
		{
			FAIL_ASSERT();
		}

		void RHI_CommandList::EndTimeBlock()
		{
			FAIL_ASSERT();
		}

		bool RHI_CommandList::CanDraw()
		{
			IS_PROFILE_FUNCTION();
			///u64 hash = 0;
			///u64 activeHash = 0;
			///{
			///	IS_PROFILE_SCOPE("Get hash");
			///	hash = m_pso.GetHash();
			///	activeHash = m_activePSO.GetHash();
			///}
			///if (hash != activeHash)
			///{
			///	m_activePSO = m_pso;
			///	RHI_DescriptorLayout* layout = m_context->GetDescriptorLayoutManager().GetLayout(m_activePSO.Shader->GetDescriptors()
			///	/*cmdList.GetDescriptorBuffer().GetDescriptorsSignature()*/);
			///	{
			///		IS_PROFILE_SCOPE("Bind pipeline");
			///		BindPipeline(m_activePSO, layout);
			///	}
			///}
			return BindDescriptorSets();
		}

		bool RHI_CommandList::BindDescriptorSets()
		{
			return false;
		}


		//// <summary>
		//// RHI_CommandListAllocator
		//// </summary>
		//// <returns></returns>
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
			std::lock_guard lock(m_lock);

			if (m_allocLists.find(cmdList) == m_allocLists.end())
			{
				IS_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is not in the allocated list. Command lists should be obtained by 'GetCommandList'.");
			}
			m_allocLists.erase(cmdList);

			if (m_freeLists.find(cmdList) != m_freeLists.end())
			{
				IS_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is in the free list. Command should not be returned more than once.");
				return;
			}
			m_freeLists.insert(cmdList);
		}

		//// <summary>
		//// CommandListManager
		//// </summary>
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
			ASSERT(m_allocator);
			return m_allocator->GetCommandList();
		}

		RHI_CommandList* CommandListManager::GetSingleUseCommandList()
		{
			ASSERT(m_allocator);
			return m_allocator->GetSingleSubmitCommandList();
		}

		void CommandListManager::ReturnCommandList(RHI_CommandList* cmdList)
		{
			ASSERT(m_allocator);
			m_allocator->ReturnCommandList(cmdList);
		}

		void CommandListManager::ReturnSingleUseCommandList(RHI_CommandList* cmdList)
		{
			ASSERT(m_allocator);
			m_allocator->ReturnSingleSubmitCommandList(cmdList);
		}

		void CommandListManager::Reset()
		{
			ASSERT(m_allocator);
			m_allocator->Reset();
		}
	}
}