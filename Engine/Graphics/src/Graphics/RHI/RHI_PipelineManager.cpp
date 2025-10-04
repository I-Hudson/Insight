#include "Graphics/RHI/RHI_PipelineManager.h"
#include "Graphics/RHI/RHI_Pipeline.h"
#include "Graphics/RHI/RHI_PipelineLayout.h"

#if defined(IS_VULKAN_ENABLED)
#endif
#if defined(IS_DX12_ENABLED)
#include "Graphics/RHI/DX12/RHI_PipelineCache_DX12.h"
#endif

#include "Graphics/RenderContext.h"

#include "Core/Profiler.h"
#include "Graphics/Descriptors.h"
#include "Graphics/RHI/RHI_Shader.h"

namespace Insight
{
	namespace Graphics
	{
		//-------------------------
		// RHI_PipelineLayoutManager
		//-------------------------
		RHI_PipelineCahce* RHI_PipelineCahce::New(RenderContext* context)
		{
#if defined(IS_VULKAN_ENABLED)
			//if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_PipelineCahce_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(context); }
#endif
#if defined(IS_DX12_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_PipelineCache_DX12, Insight::Core::MemoryAllocCategory::Graphics>(context); }
#endif
			return nullptr;
		}

		//-------------------------
		// RHI_PipelineLayoutManager
		//-------------------------
		RHI_PipelineLayoutManager::RHI_PipelineLayoutManager()
		{ }

		RHI_PipelineLayoutManager::~RHI_PipelineLayoutManager()
		{
			Destroy();
		}

		void RHI_PipelineLayoutManager::SetRenderContext(RenderContext* context)
		{
			m_context = context;
		}

		RHI_PipelineLayout* RHI_PipelineLayoutManager::GetOrCreateLayout(PipelineStateObject pso)
		{
			IS_PROFILE_FUNCTION();
			assert(m_context != nullptr);

			const u64 hash = HashDescriptors(pso.Shader);

			auto itr = m_layouts.find(hash);
			if (itr != m_layouts.end())
			{
				return itr->second;
			}

			RHI_PipelineLayout* layout = RHI_PipelineLayout::New();
			layout->Create(m_context, pso);
			m_layouts[hash] = layout;
			return layout;
		}

		RHI_PipelineLayout* RHI_PipelineLayoutManager::GetOrCreateLayout(ComputePipelineStateObject pso)
		{
			IS_PROFILE_FUNCTION();
			assert(m_context != nullptr);

			const u64 hash = HashDescriptors(pso.Shader);

			auto itr = m_layouts.find(hash);
			if (itr != m_layouts.end())
			{
				return itr->second;
			}

			RHI_PipelineLayout* layout = RHI_PipelineLayout::New();
			layout->Create(m_context, pso);
			m_layouts[hash] = layout;
			return layout;
		}

		void RHI_PipelineLayoutManager::Destroy()
		{
			IS_PROFILE_FUNCTION();

			for (auto& pair : m_layouts)
			{
				pair.second->Release();
				Delete(pair.second);
			}
			m_layouts.clear();
		}

		u64 RHI_PipelineLayoutManager::HashDescriptors(const RHI_Shader* shader) const
		{
			ASSERT(shader);

			u64 hash = 0;
			{
				IS_PROFILE_SCOPE("DescriptorSet hashing");
				const std::vector<DescriptorSet> descriptor_sets = shader->GetDescriptorSets();
				for (const DescriptorSet& descriptor_set : descriptor_sets)
				{
					HashCombine(hash, descriptor_set.GetHash(false));
				}
			}

			{
				IS_PROFILE_SCOPE("PushConstant hashing");
				PushConstant push_constant = shader->GetPushConstant();
				HashCombine(hash, push_constant.ShaderStages);
				HashCombine(hash, push_constant.Offset);
				HashCombine(hash, push_constant.Size);
			}
			return hash;
		}

		//-------------------------
		// RHI_PipelineManager
		//-------------------------
		RHI_PipelineManager::RHI_PipelineManager()
		{
			m_pipelineCache = RHI_PipelineCahce::New(m_context);
		}
		
		RHI_PipelineManager::~RHI_PipelineManager()
		{
			Destroy();
			Delete(m_pipelineCache);
		}

		void RHI_PipelineManager::SetRenderContext(RenderContext* context)
		{
			m_context = context;
		}

		RHI_Pipeline* RHI_PipelineManager::GetOrCreatePSO(PipelineStateObject pso)
		{
			IS_PROFILE_FUNCTION();

			assert(m_context != nullptr);

			const u64 psoHash = pso.GetHash();
			auto itr = m_pipelineStateObjects.find(psoHash);
			if (itr != m_pipelineStateObjects.end())
			{
				return itr->second;
			}

			pso.Shader = RenderContext::Instance().GetShaderManager().GetOrCreateShader(pso.ShaderDescription);

			RHI_Pipeline* pipeline = RHI_Pipeline::New();
			pipeline->ShaderDesc = pso.ShaderDescription;
			pipeline->Create(m_context, pso);
			m_pipelineStateObjects[psoHash] = pipeline;
			return pipeline;
		}

		RHI_Pipeline* RHI_PipelineManager::GetOrCreatePSO(ComputePipelineStateObject pso)
		{
			IS_PROFILE_FUNCTION();

			assert(m_context != nullptr);

			const u64 psoHash = pso.GetHash();
			auto itr = m_pipelineStateObjects.find(psoHash);
			if (itr != m_pipelineStateObjects.end())
			{
				return itr->second;
			}

			RHI_Pipeline* pipeline = RHI_Pipeline::New();
			pipeline->Create(m_context, pso);
			m_pipelineStateObjects[psoHash] = pipeline;
			return pipeline;
		}

		void RHI_PipelineManager::Destroy()
		{
			IS_PROFILE_FUNCTION();
			for (auto& pair : m_pipelineStateObjects)
			{
				pair.second->Release();
				Delete(pair.second);
			}
			m_pipelineStateObjects.clear();
		}

		void RHI_PipelineManager::DestroyPipelineWithShader(const ShaderDesc& shaderDesc)
		{
			m_context->GpuWaitForIdle();
			std::vector<u64> hashes;
			for (auto& [hash, pipeline] : m_pipelineStateObjects)
			{
				if (pipeline->ShaderDesc.GetHash() == shaderDesc.GetHash())
				{
					pipeline->Release();
					Delete(pipeline);
					hashes.push_back(hash);
				}
			}

			for (size_t i = 0; i < hashes.size(); ++i)
			{
				m_pipelineStateObjects.erase(hashes[i]);
			}
		}
	}
}