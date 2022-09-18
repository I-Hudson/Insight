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
		//// <summary>
		//// RH_DescriptorLayout
		//// </summary>
		//// <returns></returns>
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

		//// <summary>
		//// RHI_DescriptorLayoutManager
		//// </summary>
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
			int set = -1;

			for (const Descriptor& descriptor : descriptors)
			{
				if (descriptor.Type != DescriptorType::Unknown)
				{
					if (set == -1)
					{
						set = descriptor.Set;
					}

					HashCombine(hash, descriptor.GetHash(false));
					ASSERT(set == descriptor.Set);
				}
			}

			auto itr = m_layouts.find(hash);
			if (itr != m_layouts.end())
			{
				return itr->second;
			}

			RHI_DescriptorLayout* newLayout = RHI_DescriptorLayout::New();
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


		//// <summary>
		//// RHI_DescriptorSet
		//// </summary>
		//// <param name="context"></param>
		//// <param name="descriptors"></param>
		//// <param name="layout"></param>
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
				if (d.Type != DescriptorType::Unknown)
				{
					HashCombine(hash, d.GetHash(true));
				}
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
					if (descriptor.Type == DescriptorType::Unknown)
					{
						continue;
					}

					bool add_write = false;

					vk::WriteDescriptorSet writeDescriptorSet = { };
					writeDescriptorSet.dstSet = *reinterpret_cast<VkDescriptorSet*>(&m_resource);
					writeDescriptorSet.dstBinding = descriptor.Binding;
					writeDescriptorSet.dstArrayElement = 0;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.descriptorType = DescriptorTypeToVulkan(descriptor.Type);

					if (descriptor.Type == DescriptorType::Sampler)
					{
						const RHI_Sampler* rhi_sampler = descriptor.Sampler;
						vk::Sampler sampler_vulkan = *reinterpret_cast<const vk::Sampler*>(&rhi_sampler->Resource);

						imageInfo[imageInfoIndex].imageView = vk::ImageView();
						imageInfo[imageInfoIndex].imageLayout = vk::ImageLayout::eUndefined;
						imageInfo[imageInfoIndex].sampler = sampler_vulkan;
						writeDescriptorSet.pImageInfo = &imageInfo[imageInfoIndex];
						++imageInfoIndex;
						add_write = true;
					}

					if (descriptor.Type == DescriptorType::Sampled_Image)
					{
						imageInfo[imageInfoIndex].imageView = static_cast<const RHI::Vulkan::RHI_Texture_Vulkan*>(descriptor.Texture)->GetImageView();
						imageInfo[imageInfoIndex].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
						imageInfo[imageInfoIndex].sampler = vk::Sampler();
						writeDescriptorSet.pImageInfo = &imageInfo[imageInfoIndex];
						++imageInfoIndex;
						add_write = true;
					}

					if (descriptor.Type == DescriptorType::Combined_Image_Sampler)
					{
						const RHI_Sampler* rhi_sampler = descriptor.Sampler == nullptr ?
							m_context->GetSamplerManager().GetOrCreateSampler({}) : descriptor.Sampler;
						vk::Sampler sampler_vulkan = *reinterpret_cast<const vk::Sampler*>(&rhi_sampler->Resource);

						imageInfo[imageInfoIndex].imageView = static_cast<const RHI::Vulkan::RHI_Texture_Vulkan*>(descriptor.Texture)->GetImageView();
						imageInfo[imageInfoIndex].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
						imageInfo[imageInfoIndex].sampler = sampler_vulkan;
						writeDescriptorSet.pImageInfo = &imageInfo[imageInfoIndex];
						++imageInfoIndex;
						add_write = true;
					}

					if (descriptor.Type == DescriptorType::Unifom_Buffer)
					{
						RHI::Vulkan::RHI_Buffer_Vulkan* buffer_vulkan = descriptor.BufferView.IsValid() ?
							static_cast<RHI::Vulkan::RHI_Buffer_Vulkan*>(descriptor.BufferView.GetBuffer()) : nullptr;
						if (buffer_vulkan != nullptr)
						{
							bufferInfo[bufferInfoIndex].buffer = buffer_vulkan ? buffer_vulkan->GetBuffer() : nullptr;
							bufferInfo[bufferInfoIndex].offset = descriptor.BufferView.GetOffset();
							bufferInfo[bufferInfoIndex].range = descriptor.BufferView.GetSize();
							writeDescriptorSet.pBufferInfo = &bufferInfo[bufferInfoIndex];
							++bufferInfoIndex;
							add_write = true;
						}
					}

					if (add_write)
					{
						writes[writeIndex] = std::move(writeDescriptorSet);
						++writeIndex;
					}
				}

				if (hash != m_currentDescriptorHash)
				{
					m_currentDescriptorHash = hash;
					contextVulkan->GetDevice().updateDescriptorSets(writeIndex, &writes[0], 0, nullptr, {});
					RenderStats::Instance().DescriptorSetUpdates++;
				}
			}
#endif
#ifdef IS_DX12_ENABLED
			if (GraphicsManager::IsDX12())
			{

			}
#endif
		}


		//// <summary>
		//// RHI_DescriptorSetManager
		//// </summary>
		//// <param name="descriptors"></param>
		//// <returns></returns>
		RHI_DescriptorSet* RHI_DescriptorSetManager::GetSet(const std::vector<Descriptor>& descriptors)
		{
			IS_PROFILE_FUNCTION();

			u64 hash = 0;
			u64 hashWithResource = 0;
			for (const auto& d : descriptors)
			{
				if (d.Type != DescriptorType::Unknown)
				{
					HashCombine(hash, d.GetHash(false));
					HashCombine(hashWithResource, d.GetHash(true));
				}
			}

			/// Is there a set which is already in use with the same resources reuse that set.
			if (auto itr = m_usedSets.find(hash); itr != m_usedSets.end())
			{
				if (auto itrWithResource = itr->second.find(hashWithResource); 
					itrWithResource != itr->second.end())
				{
					return itrWithResource->second;
				}
			}

			/// No set in use with the same resources, try and find a new set.
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


		//// <summary>
		//// DescriptorAllocator
		//// </summary>
		//// <param name="pso"></param>
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
				m_descriptors[desc.Set].push_back(desc);
			}
			for (auto& descs : m_descriptors)
			{
				std::sort(descs.second.begin(), descs.second.end(), [](const Descriptor& d1, const Descriptor& d2)
					{
						return d1.Binding < d2.Binding;
					});
			}

			/// Fill in missing descriptors.
			for (auto& pair : m_descriptors)
			{
				if (pair.second.size() > 0)
				{
					u32 first_binding = pair.second.at(0).Binding;
					for (size_t i = 0; i < first_binding; ++i)
					{
						pair.second.insert(pair.second.begin(), Descriptor());
					}
				}
			}
		}

		void DescriptorAllocator::SetUniform(u32 set, u32 binding, const void* data, u32 size)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[DescriptorAllocator::SetUniform] Binding: '{0}' is out of range.", binding);
				return;
			}
			Descriptor& descriptor = descriptors.at(binding);
			if (descriptor.Size != size)
			{
				IS_CORE_ERROR("[DescriptorAllocator::SetUniform] Size mismatch. Descriptor expects '{0}', provided '{1}'\n Set: {2}, Binding: {3}."
					, descriptor.Size, size, set, binding);
			}

			CreateUniformBufferIfNoExist();
			
			RHI_BufferView view = m_uniformBuffer->Upload(data, static_cast<int>(size), static_cast<int>(m_uniformBufferOffset));
			m_uniformBufferOffset += size;

			/// Align the size to minUniformBufferOffsetAlignment.
			const u64 mask = PhysicalDeviceInformation::Instance().MinUniformBufferAlignment - 1;
			m_uniformBufferOffset = m_uniformBufferOffset + (-m_uniformBufferOffset & mask);

			descriptor.BufferView = view;
		}

		void DescriptorAllocator::SetTexture(u32 set, u32 binding, const RHI_Texture* texture, const RHI_Sampler* sampler)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[GPUDescriptorAllocator::SetTexture] Binding: '{0}' is out of range.", binding);
				return;
			}
			descriptors[binding].Texture = texture;
			descriptors[binding].Sampler = sampler != nullptr ? sampler : nullptr;
		}

		void DescriptorAllocator::SetSampler(u32 set, u32 binding, const RHI_Sampler* sampler)
		{
			std::vector<Descriptor>& descriptors = m_descriptors[set];
			if (binding >= (int)descriptors.size())
			{
				IS_CORE_ERROR("[GPUDescriptorAllocator::SetSampler] Binding: '{0}' is out of range.", binding);
				return;
			}
			descriptors[binding].Sampler = sampler;
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
				m_uniformBuffer->SetName(L"Descriptor_Uniform_Buffer");
			}
		}
	}
}