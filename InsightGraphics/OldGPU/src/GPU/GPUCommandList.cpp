#include "Graphics/GPU/GPUCommandList.h"
#include "Graphics/GPU/GPUBuffer.h"
#include "Graphics/GPU/GPUFence.h"
#include "Core/Logger.h"

#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"
#include "Graphics/GPU/RHI/DX12/GPUCommandList_DX12.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		void GPUCommandList::CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst)
		{
			const u64 srcSize = src->GetSize();
			const u64 dstSize = dst->GetSize();
			assert(srcSize == dstSize);
			CopyBufferToBuffer(src, dst, 0, 0, srcSize);
		}

		void GPUCommandList::Submit(GPUQueue queue)
		{
			Submit(queue, {}, {});
		}

		void GPUCommandList::SubmitAndWait(GPUQueue queue)
		{
			SubmitAndWait(queue, {}, {});
		}

		void GPUCommandList::SubmitAndWait(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores)
		{
			Submit(queue, waitSemaphores, signalSemaphores);
			m_fence->Wait();
		}

		void GPUCommandList::SetShader(GPUShader* shader)
		{
			m_pso.Shader = shader;
		}

		void GPUCommandList::AddRenderTarget(RenderTarget* renderTarget)
		{
			m_pso.RenderTargets.push_back(renderTarget);
		}

		void GPUCommandList::ClearRenderTargets()
		{
			m_pso.RenderTargets.clear();
		}

		void GPUCommandList::SetPrimitiveTopologyType(PrimitiveTopologyType primitiveTopologyType)
		{
			m_pso.PrimitiveTopologyType = primitiveTopologyType;
		}

		void GPUCommandList::SetPolygonMode(PolygonMode polygonMode)
		{
			m_pso.PolygonMode = polygonMode;
		}

		void GPUCommandList::SetCullMode(CullMode cullMode)
		{
			m_pso.CullMode = cullMode;
		}

		void GPUCommandList::SetFrontFace(FrontFace frontFace)
		{
			m_pso.FrontFace = frontFace;
		}

		void GPUCommandList::SetSwapchainSubmit(bool swapchainSubmit)
		{
			m_pso.Swapchain = swapchainSubmit;
		}

		bool GPUCommandList::CanDraw()
		{
			if (!m_activeItems.Renderpass)
			{
				BeginRenderpass();
			}

			if (m_activeItems.ActivePso.GetHash() != m_pso.GetHash())
			{
				// PSO is different, find out new PSO and bid it.
				GPUPipelineStateObject* pipelineStateObject = GPUPipelineStateObjectManager::Instance().GetOrCreatePSO(m_pso);
				if (!pipelineStateObject)
				{
					//IS_CORE_ERROR("[GPUCommandList::CanDraw] PipelineStateObejct is not valid. Missing some required data.");
					return false;
				}

				BindPipeline(pipelineStateObject);
				m_activeItems.ActivePso = m_pso;
			}

			// Do we have an active pipeline?
				// Do we have an active renderpass? This should be all contained within GPUPipeline.
				// we shouldn't need to have a separate GPURenderpass for this as renderpass
				// should be cached within GPUPipeline.
			// Bind all our descriptor.

			return true;
		}

		void GPUCommandList::Reset()
		{
			m_pso = {};
			m_activeItems = {};
			m_recordCommandCount = 0;
			m_fence->Reset();
		}



		GPUComamndListAllocator* GPUComamndListAllocator::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUComamndListAllocator_Vulkan(); }
			else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUComamndListAllocator_DX12(); }
			return nullptr;
		}



		GPUCommandListManager::GPUCommandListManager()
		{
		}

		GPUCommandListManager::~GPUCommandListManager()
		{
			Destroy();
		}

		void GPUCommandListManager::Create()
		{
		}

		void GPUCommandListManager::ResetCommandLists(std::string key, std::list<GPUCommandList*> cmdLists)
		{
			if (!m_commandListGroup[key].m_commandListAllocator)
			{
				return;
			}
			m_commandListGroup[key].m_commandListAllocator->ResetCommandLists(cmdLists);
		}

		void GPUCommandListManager::ResetCommandPool(std::string key, GPUCommandListType type)
		{
			if (!m_commandListGroup[key].m_commandListAllocator)
			{
				return;
			}

			m_commandListGroup[key].m_commandListAllocator->ResetCommandPool(type);
			for (auto& k : m_commandListGroup[key].m_typeToListLookup[type])
			{
				auto itr = std::find(m_commandListGroup[key].m_inUseCommandLists.begin(), m_commandListGroup[key].m_inUseCommandLists.end(), k);
				if (itr != m_commandListGroup[key].m_inUseCommandLists.end())
				{
					//GPUCommandList* cmdList = *itr;
					m_commandListGroup[key].m_freeCommandLists.push_back(*itr);
					m_commandListGroup[key].m_inUseCommandLists.erase(itr);
				}
			}
		}

		GPUCommandList* GPUCommandListManager::GetOrCreateCommandList(std::string key, GPUCommandListType type)
		{
			GPUCommandListGroup& group = m_commandListGroup[key];

			if (!group.m_commandListAllocator)
			{
				group.m_commandListAllocator = GPUComamndListAllocator::New();
			}

			if (group.m_freeCommandLists.size() > 0)
			{
				GPUCommandList* cmdList = group.m_freeCommandLists.back();
				group.m_freeCommandLists.pop_back();
				group.m_inUseCommandLists.push_back(cmdList);
				return cmdList;
			}

			GPUCommandList* cmdList = group.m_commandListAllocator->AllocateCommandList(type);
			group.m_inUseCommandLists.push_back(cmdList);
			group.m_typeToListLookup[type].push_back(cmdList);
			return cmdList;
		}

		void GPUCommandListManager::ReturnCommandList(std::string key, GPUCommandList* cmdList)
		{
			std::list<GPUCommandList*>::iterator itr = std::find(m_commandListGroup[key].m_inUseCommandLists.begin(), m_commandListGroup[key].m_inUseCommandLists.end(), cmdList);
			if (itr != m_commandListGroup[key].m_inUseCommandLists.end())
			{
				m_commandListGroup[key].m_inUseCommandLists.erase(itr);
			}
			else
			{
				// ERROR: Command list not being tracked.
			}

			itr = std::find(m_commandListGroup[key].m_freeCommandLists.begin(), m_commandListGroup[key].m_freeCommandLists.end(), cmdList);
			if (itr != m_commandListGroup[key].m_freeCommandLists.end())
			{
				// ERROR: Command list already in free list.
				return;
			}
			m_commandListGroup[key].m_freeCommandLists.push_back(cmdList);
		}

		void GPUCommandListManager::Destroy()
		{
			for (auto& pair : m_commandListGroup)
			{
				if (pair.second.m_inUseCommandLists.size() > 0)
				{
					// ERROR: Command lists are in use.
					IS_CORE_WARN("[GPUCommandListManager::Destroy] Command list are still in use.");
				}

				pair.second.m_commandListAllocator->FreeCommandLists(pair.second.m_inUseCommandLists);
				pair.second.m_commandListAllocator->FreeCommandLists(pair.second.m_freeCommandLists);
				pair.second.m_inUseCommandLists.resize(0);
				pair.second.m_freeCommandLists.resize(0);
				pair.second.m_typeToListLookup.clear();


				pair.second.m_commandListAllocator->Destroy();
				delete pair.second.m_commandListAllocator;
				pair.second.m_commandListAllocator = nullptr;
			}
			m_commandListGroup.clear();
		}

		const std::list<GPUCommandList*>& GPUCommandListManager::GetAllInUseCommandLists(std::string key) const
		{
			return m_commandListGroup.at(key).m_inUseCommandLists;
		}
	}
}