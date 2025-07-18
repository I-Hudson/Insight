#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RenderContext.h"

#if defined(IS_VULKAN_ENABLED)
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#endif
#if defined(IS_DX12_ENABLED)
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#endif

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
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_CommandList_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#if defined(IS_DX12_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_CommandList_DX12, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
			return nullptr;
		}

		RHI_CommandListStates RHI_CommandList::GetState() const
		{
			return m_state;
		}

		void RHI_CommandList::Reset()
		{
			m_pso = {};
			m_activePSO = {};
			m_drawData = {};
			m_boundVertexBufferViews.clear();
			m_boundIndexBufferView = { };
			m_discard = false;
		}

		void RHI_CommandList::CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src)
		{
			ASSERT(src->GetSize() <= dst->GetSize());
			CopyBufferToBuffer(dst, 0, src, 0, src->GetSize());
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

				//for (size_t i = 0; i < create_info.Mip_Count; ++i)
				//{
				//	texture->m_infos[i].Layout = layout;
				//}
			}
		}

		void RHI_CommandList::SetUniform(u32 set, u32 binding, const void* data, u32 size)
		{
			m_descriptorAllocator->SetUniform(set, binding, data, size);
		}
		void RHI_CommandList::SetTexture(u32 set, u32 binding, const RHI_Texture* texture)
		{
			m_descriptorAllocator->SetTexture(set, binding, texture);
			m_context->GetResourceRenderTracker().TrackResource(texture);
		}
		void RHI_CommandList::SetSampler(u32 set, u32 binding, const RHI_Sampler* sampler)
		{
			m_descriptorAllocator->SetSampler(set, binding, sampler);
		}
		void RHI_CommandList::SetUnorderedAccess(const u32 set, const u32 binding, const RHI_BufferView bufferView)
		{
			m_descriptorAllocator->SetUnorderedAccess(set, binding, bufferView);
			m_context->GetResourceRenderTracker().TrackResource(bufferView.GetBuffer());
		}
		void RHI_CommandList::SetUnorderedAccess(const u32 set, const u32 binding, const RHI_Texture* texture)
		{
			m_descriptorAllocator->SetUnorderedAccess(set, binding, texture);
			m_context->GetResourceRenderTracker().TrackResource(texture);
		}

		RHI_BufferView RHI_CommandList::UploadUniform(const void* data, u32 size)
		{
			return m_descriptorAllocator->UploadUniform(data, size);
		}

		void RHI_CommandList::SetUniform(u32 set, u32 binding, RHI_BufferView buffer)
		{
			m_descriptorAllocator->SetUniform(set, binding, buffer);
		}

		void RHI_CommandList::BeginTimeBlock(const std::string& blockName)
		{
			FAIL_ASSERT();
		}

		void RHI_CommandList::BeginTimeBlock(const std::string& blockName, Maths::Vector4 colour)
		{
			FAIL_ASSERT();
		}

		void RHI_CommandList::EndTimeBlock()
		{
			FAIL_ASSERT();
		}

		bool RHI_CommandList::CanDraw(const GPUQueue gpuQueue)
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

			// Make sure our buffers are good to go.
			{
				IS_PROFILE_SCOPE("Verify VertexBuffers");
				for (const RHI_BufferView& view : m_boundVertexBufferViews)
				{
					if (view.IsValid() && view.GetBuffer()->GetUploadStatus() != DeviceUploadStatus::Completed)
					{
						return false;
					}
				}
			}
			{
				IS_PROFILE_SCOPE("Verify IndexBuffers");
				if (m_boundIndexBufferView.IsValid() && m_boundIndexBufferView.GetBuffer()->GetUploadStatus() != DeviceUploadStatus::Completed)
				{
					return false;
				}
			}
			 
			return BindDescriptorSets(gpuQueue);
		}


		//// <summary>
		//// RHI_CommandListAllocator
		//// </summary>
		//// <returns></returns>
		RHI_CommandListAllocator* RHI_CommandListAllocator::New()
		{
#if defined(IS_VULKAN_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_CommandListAllocator_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#if defined(IS_DX12_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_CommandListAllocator_DX12, Insight::Core::MemoryAllocCategory::Graphics>();}
#endif
			return nullptr;
		}

		u32 RHI_CommandListAllocator::FreeSize() const
		{
			std::lock_guard lock(m_mutex);
			return static_cast<u32>(m_freeLists.size());
		}

		bool RHI_CommandListAllocator::ReturnCommandList(RHI_CommandList* cmdList)
		{
			std::lock_guard lock(m_mutex);

			if (m_allocLists.find(cmdList) == m_allocLists.end())
			{
				//IS_LOG_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is not in the allocated list. Command lists should be obtained by 'GetCommandList'.");
				return false;
			}

			m_allocLists.erase(cmdList);
			if (m_freeLists.find(cmdList) != m_freeLists.end())
			{
				FAIL_ASSERT_MSG("[RHI_CommandListAllocator::ReturnCommandList] CommandList is in the free list. Command should not be returned more than once.");
			}
			else
			{
				m_freeLists.insert(cmdList);
			}
			return true;
		}

		//// <summary>
		//// CommandListManager
		//// </summary>
		CommandListManager::CommandListManager()
		{
		}

		CommandListManager::CommandListManager(CommandListManager&& other)
			: m_context(std::move(other.m_context))
			, m_commandListAllocatorDesc(std::move(other.m_commandListAllocatorDesc))
			, m_currentAllocator(std::move(other.m_currentAllocator))
			, m_allocators(std::move(other.m_allocators))
		{
			other.m_context = nullptr;
			other.m_commandListAllocatorDesc = {};
			other.m_currentAllocator = {};
			m_allocators = {};
		}

		CommandListManager::~CommandListManager()
		{
		}

		void CommandListManager::Create(RenderContext* context)
		{
			{
				std::lock_guard lock(m_mutex);
				m_context = context;
			}
			AddNewAllocator();
		}

		void CommandListManager::Update()
		{
			std::lock_guard lock(m_mutex);
			for (RHI_CommandListAllocator* allocator : m_allocators)
			{
				allocator->Reset();
			}
		}

		void CommandListManager::Destroy()
		{
			std::lock_guard lock(m_mutex);
			for (RHI_CommandListAllocator*& allocator : m_allocators)
			{
				allocator->Release();
				Delete(allocator);
			}
			m_allocators.clear();
		}

		RHI_CommandList* CommandListManager::GetCommandList()
		{
			{
				std::lock_guard lock(m_mutex);
				for (RHI_CommandListAllocator* allocator : m_allocators)
				{
					if (allocator->FreeSize() > 0)
					{
						return allocator->GetCommandList();
					}
				}
			}
			AddNewAllocator();
			{
				std::lock_guard lock(m_mutex);
				return m_allocators.back()->GetCommandList();
			}
		}

		void CommandListManager::ReturnCommandList(RHI_CommandList* cmdList)
		{
			std::lock_guard lock(m_mutex);
			for (RHI_CommandListAllocator* allocator : m_allocators)
			{
				if (allocator->ReturnCommandList(cmdList))
				{
					return;
				}
			}
		}

		void CommandListManager::Reset()
		{
			std::lock_guard lock(m_mutex);
			for (RHI_CommandListAllocator* allocator : m_allocators)
			{
				allocator->Reset();
			}
		}

		void CommandListManager::AddNewAllocator()
		{
			RHI_CommandListAllocator* newAllocator = RHI_CommandListAllocator::New();
			newAllocator->Create(m_context, m_commandListAllocatorDesc);
			newAllocator->SetName("Command Allocator_" + std::to_string(m_allocators.size()));
			{
				std::lock_guard lock(m_mutex);
				m_allocators.push_back(newAllocator);
			}
		}
	}
}