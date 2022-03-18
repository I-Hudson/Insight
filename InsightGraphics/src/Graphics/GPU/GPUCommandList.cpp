#include "Graphics/GPU/GPUCommandList.h"
#include "Graphics/GPU/GPUBuffer.h"
#include "Graphics/GPU/GPUFence.h"

#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"

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
			Submit(queue, {}, {}, nullptr);
		}

		void GPUCommandList::SubmitAndWait(GPUQueue queue)
		{
			SubmitAndWait(queue, {}, {}, nullptr);
		}

		void GPUCommandList::SubmitAndWait(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence)
		{
			GPUFence* waitFence = fence != nullptr ? fence : GPUFenceManager::Instance().GetFence();
			Submit(queue, waitSemaphores, signalSemaphores, waitFence);
			waitFence->Wait();
			if (fence == nullptr)
			{
				GPUFenceManager::Instance().ReturnFence(waitFence);
			}
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
					//std::cout << "[GPUCommandList::CanDraw] PipelineStateObejct is not valid. Missing some required data.\n";
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
			if (!m_commandListAllocator)
			{
				m_commandListAllocator = new RHI::Vulkan::GPUComamndListAllocator_Vulkan();
			}
		}

		void GPUCommandListManager::SetQueue(GPUQueue queue)
		{
			m_commandListAllocator->SetQueue(queue);
		}

		void GPUCommandListManager::ResetCommandList(GPUCommandList* cmdList)
		{
			ResetCommandLists({cmdList});
		}

		void GPUCommandListManager::ResetCommandLists(std::list<GPUCommandList*> cmdLists)
		{
			m_commandListAllocator->ResetCommandLists(cmdLists);
		}

		void GPUCommandListManager::ResetCommandPool(GPUCommandListType type)
		{
			m_commandListAllocator->ResetCommandPool(type);
			for (auto& k : m_typeToListLookup[type])
			{
				auto itr = std::find(m_inUseCommandLists.begin(), m_inUseCommandLists.end(), k);
				if (itr != m_inUseCommandLists.end())
				{
					//GPUCommandList* cmdList = *itr;
					m_freeCommandLists.push_back(*itr);
					m_inUseCommandLists.erase(itr);
				}
			}
		}

		GPUCommandList* GPUCommandListManager::GetOrCreateCommandList(GPUCommandListType type)
		{
			if (m_freeCommandLists.size() > 0)
			{
				GPUCommandList* cmdList = m_freeCommandLists.back();
				m_freeCommandLists.pop_back();
				m_inUseCommandLists.push_back(cmdList);
				return cmdList;
			}

			GPUCommandList* cmdList = m_commandListAllocator->AllocateCommandList(type);
			m_inUseCommandLists.push_back(cmdList);
			m_typeToListLookup[type].push_back(cmdList);
			return cmdList;
		}

		void GPUCommandListManager::ReturnCommandList(GPUCommandList* cmdList)
		{
			std::list<GPUCommandList*>::iterator itr = std::find(m_inUseCommandLists.begin(), m_inUseCommandLists.end(), cmdList);
			if (itr != m_inUseCommandLists.end())
			{
				m_inUseCommandLists.erase(itr);
			}
			else
			{
				// ERROR: Command list not being tracked.
			}

			itr = std::find(m_freeCommandLists.begin(), m_freeCommandLists.end(), cmdList);
			if (itr != m_freeCommandLists.end())
			{
				// ERROR: Command list already in free list.
				return;
			}
			m_freeCommandLists.push_back(cmdList);
		}

		void GPUCommandListManager::Destroy()
		{
			if (!m_commandListAllocator)
			{
				return;
			}

			if (m_inUseCommandLists.size() > 0)
			{
				// ERROR: Command lists are in use.
			}
			m_commandListAllocator->FreeCommandLists(m_inUseCommandLists);
			m_commandListAllocator->FreeCommandLists(m_freeCommandLists);
			m_inUseCommandLists.resize(0);
			m_freeCommandLists.resize(0);
			m_typeToListLookup.clear();

			m_commandListAllocator->Destroy();
			delete m_commandListAllocator;
			m_commandListAllocator = nullptr;
		}
	}
}