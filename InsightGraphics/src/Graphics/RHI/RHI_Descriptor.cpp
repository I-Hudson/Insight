#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/RHI_Shader.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/RenderContext.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"

#if defined(IS_VULKAN_ENABLED)
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#endif

#include "Core/Logger.h"
#include "Core/Profiler.h"

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
#if defined(IS_VULKAN_ENABLED)
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_DescriptorLayout_Vulkan); }
#endif
#if defined(IS_DX12_ENABLED)
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_DescriptorLayout_DX12); }
#endif	
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

		RHI_DescriptorLayout* RHI_DescriptorLayoutManager::GetLayout(std::vector<Descriptor> descriptors)
		{
			IS_PROFILE_FUNCTION();

			u64 hash = 0;
			int set = 0;

			if (!descriptors.empty())
			{
				set = descriptors.at(0).Set;
			}

			for (const Descriptor& descriptor : descriptors)
			{
				HashCombine(hash, descriptor.GetHash(false));
				ASSERT(set == descriptor.Set);
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
		/// RHI_DescriptorSet
		/// </summary>
		/// <param name="context"></param>
		/// <param name="descriptors"></param>
		/// <param name="layout"></param>
		RHI_DescriptorSet::RHI_DescriptorSet(RenderContext* context, const std::vector<Descriptor>& descriptors, RHI_DescriptorLayout* layout)
		{
			m_context = context;
			Create(layout);
			Update(descriptors);
		}

		void RHI_DescriptorSet::Release()
		{
#ifdef IS_VULKAN_ENABLED
			if (GraphicsManager::IsVulkan())
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);
				contextVulkan->GetDevice().freeDescriptorSets(*reinterpret_cast<vk::DescriptorPool*>(&contextVulkan->m_descriptor_pool)
					, { *reinterpret_cast<vk::DescriptorSet*>(&m_resource) });
			}
#endif
#ifdef IS_DX12_ENABLED
			if (GraphicsManager::IsDX12())
			{

			}
#endif
			m_resource = nullptr;
		}

		bool RHI_DescriptorSet::ValidResouce()
		{
			return m_resource != nullptr;
		}

		void RHI_DescriptorSet::SetName(std::wstring name)
		{
		}

		void RHI_DescriptorSet::Create(RHI_DescriptorLayout* layout)
		{
#ifdef IS_VULKAN_ENABLED
			if (GraphicsManager::IsVulkan())
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);

				vk::DescriptorSetLayout setLayout = static_cast<RHI::Vulkan::RHI_DescriptorLayout_Vulkan*>(layout)->GetLayout();

				vk::DescriptorSetAllocateInfo allocInfo = {};
				allocInfo.descriptorPool = *reinterpret_cast<vk::DescriptorPool*>(&contextVulkan->m_descriptor_pool);
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &setLayout;

				std::vector<vk::DescriptorSet> sets = contextVulkan->GetDevice().allocateDescriptorSets(allocInfo);
				m_resource = *reinterpret_cast<VkDescriptorSet*>(&sets.at(0));
			}
#endif
#ifdef IS_DX12_ENABLED
			if (GraphicsManager::IsDX12())
			{

			}
#endif
		}

		void RHI_DescriptorSet::Update(const std::vector<Descriptor>& descriptors)
		{
			IS_PROFILE_FUNCTION();
		
			u64 hash = 0;
			for (const auto& d : descriptors)
			{
				HashCombine(hash, d.GetHash(true));
			}

#ifdef IS_VULKAN_ENABLED
			if (GraphicsManager::IsVulkan())
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);

				u32 bufferInfoIndex = 0;
				u32 imageInfoIndex = 0;
				u32 writeIndex = 0;

				const u32 c_MaxWrites = 32;

				vk::DescriptorBufferInfo bufferInfo[c_MaxWrites];
				vk::DescriptorImageInfo imageInfo[c_MaxWrites];
				vk::WriteDescriptorSet writes[c_MaxWrites];

				for (const Descriptor& descriptor : descriptors)
				{
					vk::WriteDescriptorSet writeDescriptorSet = { };
					writeDescriptorSet.dstSet = *reinterpret_cast<VkDescriptorSet*>(&m_resource);
					writeDescriptorSet.dstBinding = descriptor.Binding;
					writeDescriptorSet.dstArrayElement = 0;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.descriptorType = DescriptorTypeToVulkan(descriptor.Type);

					if (descriptor.Texture)
					{
						RHI_Sampler rhi_sampler = m_context->GetSamplerManager().GetOrCreateSampler({});
						vk::Sampler sampler_vulkan = *reinterpret_cast<vk::Sampler*>(&rhi_sampler);

						imageInfo[imageInfoIndex].imageView = static_cast<RHI::Vulkan::RHI_Texture_Vulkan*>(descriptor.Texture)->GetImageView();
						imageInfo[imageInfoIndex].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
						imageInfo[imageInfoIndex].sampler = sampler_vulkan;
						writeDescriptorSet.pImageInfo = &imageInfo[imageInfoIndex];
						++imageInfoIndex;
					}

					if (descriptor.BufferView.IsValid())
					{
						bufferInfo[bufferInfoIndex].buffer = static_cast<RHI::Vulkan::RHI_Buffer_Vulkan*>(descriptor.BufferView.GetBuffer())->GetBuffer();	
						bufferInfo[bufferInfoIndex].offset = descriptor.BufferView.GetOffset();
						bufferInfo[bufferInfoIndex].range = descriptor.BufferView.GetSize();
						
							writeDescriptorSet.pBufferInfo = &bufferInfo[bufferInfoIndex];
						++bufferInfoIndex;
					}

					writes[writeIndex] = std::move(writeDescriptorSet);
					++writeIndex;
				}

				if (hash != m_currentDescriptorHash)
				{
					m_currentDescriptorHash = hash;
					contextVulkan->GetDevice().updateDescriptorSets(writeIndex, &writes[0], 0, nullptr, {});
				}
			}
#endif
#ifdef IS_DX12_ENABLED
			if (GraphicsManager::IsDX12())
			{

			}
#endif
		}


		/// <summary>
		/// RHI_DescriptorSetManager
		/// </summary>
		/// <param name="descriptors"></param>
		/// <returns></returns>
		RHI_DescriptorSet* RHI_DescriptorSetManager::GetSet(const std::vector<Descriptor>& descriptors)
		{
			IS_PROFILE_FUNCTION();

			u64 hash = 0;
			u64 hashWithResource = 0;
			for (const auto& d : descriptors)
			{
				HashCombine(hash, d.GetHash(false));
				HashCombine(hashWithResource, d.GetHash(true));
			}

			// Is there a set which is already in use with the same resources reuse that set.
			if (auto itr = m_usedSets.find(hash); itr != m_usedSets.end())
			{
				if (auto itrWithResource = itr->second.find(hashWithResource); 
					itrWithResource != itr->second.end())
				{
					return itrWithResource->second;
				}
			}

			// No set in use with the same resources, try and find a new set.
			if (auto itr = m_freeSets.find(hash); itr != m_freeSets.end())
			{
				if (!itr->second.empty())
				{
					RHI_DescriptorSet* set = itr->second.front();
					itr->second.pop_front();
					m_usedSets[hash][hashWithResource] = set;
					set->Update(descriptors);
					return set;
				}
			}

			RHI_DescriptorSet* newSet = NewArgsTracked(RHI_DescriptorSet, GraphicsManager::Instance().GetRenderContext()
				, descriptors
				, GraphicsManager::Instance().GetRenderContext()->GetDescriptorLayoutManager().GetLayout(descriptors));

			m_usedSets[hash][hashWithResource] = newSet;

			return newSet;
		}

		void RHI_DescriptorSetManager::Reset()
		{
			for (auto& setCollection : m_usedSets)
			{
				auto& freeSetCollection = m_freeSets[setCollection.first];
				for (auto& usedCollection : setCollection.second)
				{
					freeSetCollection.push_back(usedCollection.second);
				}
			}
			m_usedSets.clear();
		}

		void RHI_DescriptorSetManager::ReleaseAll()
		{
			Reset();

			for (auto& setCollection : m_freeSets)
			{
				for (auto& set : setCollection.second)
				{
					set->Release();
					DeleteTracked(set);
				}
				setCollection.second.clear();
			}
		}


		/// <summary>
		/// DescriptorAllocator
		/// </summary>
		/// <param name="pso"></param>
		DescriptorAllocator::DescriptorAllocator()
		{ }

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
				if (desc.Type == DescriptorType::Sampler)
				{
					continue;
				}

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

		void DescriptorAllocator::SetUniform(int set, int binding, void* data, u32 size)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[GPUDescriptorAllocator::SetUniform] Binding: '{0}' is out of range.", binding);
				return;
			}
			CreateUniformBufferIfNoExist();
			
			RHI_BufferView view = m_uniformBuffer->Upload(data, static_cast<int>(size), static_cast<int>(m_uniformBufferOffset));
			m_uniformBufferOffset += size;

			// Align the size to minUniformBufferOffsetAlignment.
			const u64 mask = PhysicalDeviceInformation::Instance().MinUniformBufferAlignment - 1;
			m_uniformBufferOffset = m_uniformBufferOffset + (-m_uniformBufferOffset & mask);

			descriptors[binding].BufferView = view;
		}

		void DescriptorAllocator::SetTexture(int set, int binding, RHI_Texture* texture)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[GPUDescriptorAllocator::SetTexture] Binding: '{0}' is out of range.", binding);
				return;
			}
			descriptors[binding].Texture = texture;
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

		void DescriptorAllocator::SetRenderContext(RenderContext* context)
		{
			m_context = context;
		}

		bool DescriptorAllocator::GetDescriptorSets(std::vector<RHI_DescriptorSet*>& sets)
		{
			IS_PROFILE_FUNCTION();
			for (const auto& set : m_descriptors)
			{
				sets.push_back(m_context->GetDescriptorSetManager().GetSet(set.second));
			}
			return sets.size() > 0 ? true : false;
		}

		void DescriptorAllocator::ClearDescriptors()
		{
			m_descriptors.clear();
		}

		void DescriptorAllocator::Reset()
		{
			ClearDescriptors();
			m_uniformBufferOffset = 0;
		}

		void DescriptorAllocator::Destroy()
		{
			Renderer::FreeUniformBuffer(m_uniformBuffer.Get());
			m_uniformBuffer.Release();
		}

		void DescriptorAllocator::CreateUniformBufferIfNoExist()
		{
			if (!m_uniformBuffer)
			{
				m_uniformBuffer = UPtr(Renderer::CreateUniformBuffer(1_MB));
			}
		}
	}
}