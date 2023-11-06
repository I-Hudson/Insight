#include "Graphics/RHI/RHI_PipelineManager.h"
#include "Graphics/RHI/RHI_Pipeline.h"
#include "Graphics/RHI/RHI_PipelineLayout.h"

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

				u64 hash = 0;
			{
				IS_PROFILE_SCOPE("DescriptorSet hashing");
				const std::vector<DescriptorSet> descriptor_sets = pso.Shader->GetDescriptorSets();
				for (const DescriptorSet& descriptor_set : descriptor_sets)
				{
					HashCombine(hash, descriptor_set.GetHash(false));
				}
			}

			{
				IS_PROFILE_SCOPE("PushConstant hashing");
				PushConstant push_constant = pso.Shader->GetPushConstant();
				HashCombine(hash, push_constant.ShaderStages);
				HashCombine(hash, push_constant.Offset);
				HashCombine(hash, push_constant.Size);
			}

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
			for (auto& pair : m_layouts)
			{
				pair.second->Release();
				Delete(pair.second);
			}
			m_layouts.clear();
		}

		//-------------------------
		// RHI_PipelineManager
		//-------------------------
		RHI_PipelineManager::RHI_PipelineManager()
		{ }
		
		RHI_PipelineManager::~RHI_PipelineManager()
		{
			Destroy();
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
	}
}