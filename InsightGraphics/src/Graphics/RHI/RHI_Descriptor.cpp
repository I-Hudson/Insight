#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/RHI_Shader.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		/// <summary>
		/// RH_DescriptorLayout
		/// </summary>
		/// <returns></returns>
		RHI_DescriptorLayout* RHI_DescriptorLayout::New()
		{
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_DescriptorLayout_Vulkan); }
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_DescriptorLayout_DX12); }
			return nullptr;
		}

		/// <summary>
		/// RHI_DescriptorLayoutManager
		/// </summary>
		RHI_DescriptorLayoutManager::RHI_DescriptorLayoutManager()
		{
		}

		RHI_DescriptorLayoutManager::~RHI_DescriptorLayoutManager()
		{
			ReleaseAll();
		}

		RHI_DescriptorLayout* RHI_DescriptorLayoutManager::GetLayout(int set, std::vector<Descriptor> descriptors)
		{
			u64 hash = 0;
			for (const Descriptor& descriptor : descriptors)
			{
				HashCombine(hash, descriptor.GetHash(false));
			}

			auto itr = m_layouts.find(hash);
			if (itr != m_layouts.end())
			{
				return itr->second;
			}

			RHI_DescriptorLayout* newLayout = RHI_DescriptorLayout::New();
			//assert(newLayout);
			m_layouts[hash] = newLayout;
			if (newLayout)
			{
				newLayout->Create(m_context, set, descriptors);
			}
			return newLayout;
		}

		void RHI_DescriptorLayoutManager::ReleaseAll()
		{
			for (auto& pair : m_layouts)
			{
				if (pair.second)
				{
					pair.second->Release();
					DeleteTracked(pair.second);
				}
			}
			m_layouts.clear();
		}


		/// <summary>
		/// RHI_Descriptor
		/// </summary>
		/// <returns></returns>
		RHI_Descriptor* RHI_Descriptor::New()
		{
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_Descriptor_Vulkan); }
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_Descriptor_DX12); }
			return nullptr;
		}



		/// <summary>
		/// DescriptorAllocator
		/// </summary>
		/// <param name="pso"></param>
		void DescriptorAllocator::SetPipeline(PipelineStateObject pso)
		{
			RHI_Shader* shader = pso.Shader;
			if (!shader)
			{
				return;
			}

			m_descriptors.clear();

			std::vector<Descriptor> descriptors = shader->GetDescriptors();
			for (const Descriptor& desc : descriptors)
			{
				m_descriptors[desc.Set].push_back(desc);
			}
			for (auto& descs : m_descriptors)
			{
				std::sort(descs.second.begin(), descs.second.end(), [](const Descriptor& d1, const Descriptor& d2)
					{
						return d1.Binding < d2.Binding;
					});
			}
		}

		void DescriptorAllocator::SetUniform(int set, int binding, RHI_BufferView view)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[GPUDescriptorAllocator::SetUniform] Binding: '{0}' is out of range.", binding);
				return;
			}
			descriptors[binding].BufferView = view;
		}


		Descriptor DescriptorAllocator::GetDescriptor(int set, int binding)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[GPUDescriptorAllocator::SetUniform] Binding: '{0}' is out of range.", binding);
				return { };
			}
			return descriptors[binding];
		}
	}
}