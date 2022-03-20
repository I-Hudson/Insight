#include "Graphics/GPU/GPUPipelineStateObject.h"
#include "Graphics/GPU/RHI/Vulkan/GPUPipelineStateObject_Vulkan.h"
#include "Core/Logger.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUPipelineStateObject* GPUPipelineStateObject::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUPipelineStateObject_Vulkan(); }
			//else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUPipelineStateObject_DX12(); }
			return nullptr;
		}


		GPUPipelineStateObjectManager::GPUPipelineStateObjectManager()
		{
		}

		GPUPipelineStateObjectManager::~GPUPipelineStateObjectManager()
		{
			Destroy();
		}

		GPUPipelineStateObject* GPUPipelineStateObjectManager::GetOrCreatePSO(PipelineStateObject pso)
		{
			if (!pso.IsValid())
			{
				IS_CORE_ERROR("[GPUPipelineStateObjectManager::GetOrCreatePSO] PipelineStateObject is not valid.");
				return nullptr;
			}

			const u64 psoHash = pso.GetHash();
			std::map<u64, GPUPipelineStateObject*>::iterator itr = m_pipelineStateObjects.find(psoHash);
			if (itr != m_pipelineStateObjects.end() && itr->second != nullptr)
			{
				// We found a pipeline which has already been created.
				return itr->second;
			}

			GPUPipelineStateObject* psoPtr = GPUPipelineStateObject::New();
			psoPtr->Create(pso);
			m_pipelineStateObjects[psoHash] = psoPtr;
			return psoPtr;
		}

		// This should get called in GPUDevice_API
		void GPUPipelineStateObjectManager::Destroy()
		{
			for (const std::pair<u64, GPUPipelineStateObject*>& pair : m_pipelineStateObjects)
			{
				pair.second->Destroy();
				delete pair.second;
			}
			m_pipelineStateObjects.clear();
		}
	}
}