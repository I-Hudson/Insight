#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/RHI_Shader.h"


#include "Graphics/RenderContext.h"

#if defined(IS_VULKAN_ENABLED)
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#endif
#if defined(IS_DX12_ENABLED)
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

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
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_DescriptorLayout_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#if defined(IS_DX12_ENABLED)
			//else if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_DescriptorLayout_DX12, Insight::Core::MemoryAllocCategory::Graphics>(); }
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

		RHI_DescriptorLayout* RHI_DescriptorLayoutManager::GetLayout(const DescriptorSet& descriptor_set)
		{
			IS_PROFILE_FUNCTION();

			u64 hash = 0;
			int set = -1;

			for (const DescriptorBinding& descriptor : descriptor_set.Bindings)
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
				newLayout->Create(m_context, set, descriptor_set);
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
		RHI_DescriptorSet::RHI_DescriptorSet(RenderContext* context, const DescriptorSet& descriptor_set, RHI_DescriptorLayout* layout)
		{
			m_context = context;
			Create(layout);
			Update(descriptor_set);
		}

		void RHI_DescriptorSet::Release()
		{
#ifdef IS_VULKAN_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);
				vkFreeDescriptorSets(contextVulkan->GetDevice(), 
					contextVulkan->m_descriptor_pool,
					1,
					reinterpret_cast<VkDescriptorSet*>(&m_resource));
			}
#endif
#ifdef IS_DX12_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
			{

			}
#endif
			m_resource = nullptr;
		}

		bool RHI_DescriptorSet::ValidResource()
		{
			return m_resource != nullptr;
		}

		void RHI_DescriptorSet::SetName(std::string name)
		{
		}

		void RHI_DescriptorSet::Create(RHI_DescriptorLayout* layout)
		{
#ifdef IS_VULKAN_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);

				VkDescriptorSetLayout setLayout = static_cast<RHI::Vulkan::RHI_DescriptorLayout_Vulkan*>(layout)->GetLayout();

				VkDescriptorSetAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = contextVulkan->m_descriptor_pool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &setLayout;

				std::vector<VkDescriptorSet> sets;
				sets.resize(allocInfo.descriptorSetCount);
				vkAllocateDescriptorSets(contextVulkan->GetDevice(), &allocInfo, sets.data());
				m_resource = reinterpret_cast<VkDescriptorSet>(sets.at(0));
			}
#endif
#ifdef IS_DX12_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
			{

			}
#endif
		}

		void RHI_DescriptorSet::Update(const DescriptorSet& descriptor_set)
		{
			IS_PROFILE_FUNCTION();
		
			u64 hash = 0;
			for (const auto& d : descriptor_set.Bindings)
			{
				if (d.Type != DescriptorType::Unknown)
				{
					HashCombine(hash, d.GetHash(true));
				}
			}

#ifdef IS_VULKAN_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);

				u32 bufferInfoIndex = 0;
				u32 imageInfoIndex = 0;
				u32 writeIndex = 0;

				const u32 c_MaxWrites = 32;

				VkDescriptorBufferInfo bufferInfo[c_MaxWrites];
				VkDescriptorImageInfo imageInfo[c_MaxWrites];
				VkWriteDescriptorSet writes[c_MaxWrites];

				for (const DescriptorBinding& descriptor : descriptor_set.Bindings)
				{
					if (descriptor.Type == DescriptorType::Unknown)
					{
						continue;
					}

					bool add_write = false;

					VkWriteDescriptorSet writeDescriptorSet = { };
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = *reinterpret_cast<VkDescriptorSet*>(&m_resource);
					writeDescriptorSet.dstBinding = descriptor.Binding;
					writeDescriptorSet.dstArrayElement = 0;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.descriptorType = DescriptorTypeToVulkan(descriptor.Type);

					if (descriptor.Type == DescriptorType::Sampler)
					{
						if (descriptor.RHI_Sampler)
						{
							const RHI_Sampler* rhi_sampler = descriptor.RHI_Sampler;
							VkSampler sampler_vulkan = *reinterpret_cast<const VkSampler*>(&rhi_sampler->Resource);

							imageInfo[imageInfoIndex].imageView = VkImageView();
							imageInfo[imageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							imageInfo[imageInfoIndex].sampler = sampler_vulkan;
							writeDescriptorSet.pImageInfo = &imageInfo[imageInfoIndex];
							++imageInfoIndex;
							add_write = true;
						}
					}

					if (descriptor.Type == DescriptorType::Sampled_Image)
					{
						if (descriptor.RHI_Texture && descriptor.RHI_Texture->GetUploadStatus() == DeviceUploadStatus::Completed)
						{
							imageInfo[imageInfoIndex].imageView = static_cast<const RHI::Vulkan::RHI_Texture_Vulkan*>(descriptor.RHI_Texture)->GetImageView();
							imageInfo[imageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							imageInfo[imageInfoIndex].sampler = VkSampler();
							writeDescriptorSet.pImageInfo = &imageInfo[imageInfoIndex];
							++imageInfoIndex;
							add_write = true;
						}
					}

					if (descriptor.Type == DescriptorType::Unifom_Buffer)
					{
						if (descriptor.RHI_Buffer_View.IsValid())
						{
							RHI::Vulkan::RHI_Buffer_Vulkan* buffer_vulkan = static_cast<RHI::Vulkan::RHI_Buffer_Vulkan*>(descriptor.RHI_Buffer_View.GetBuffer());
							bufferInfo[bufferInfoIndex].buffer = buffer_vulkan ? buffer_vulkan->GetBuffer() : nullptr;
							bufferInfo[bufferInfoIndex].offset = descriptor.RHI_Buffer_View.GetOffset();
							bufferInfo[bufferInfoIndex].range = descriptor.RHI_Buffer_View.GetSize();
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

				// TODO: Fix this. With imgui there is an issue where with a 2 frame swapchain the second frame 
				// does not show imgui correctly as the texture binding is not set. Unsure why this is but with the 
				// below commented out it works. So something is wrong with the hash/caching and imgui is needing this
				// to be force update which is bad for performance. 
				//if (hash != m_currentDescriptorHash)
				{
					m_currentDescriptorHash = hash;
					vkUpdateDescriptorSets(contextVulkan->GetDevice(), writeIndex, &writes[0], 0, nullptr);
					RenderStats::Instance().DescriptorSetUpdates++;
				}
			}
#endif
#ifdef IS_DX12_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
			{

			}
#endif
		}


		//// <summary>
		//// RHI_DescriptorSetManager
		//// </summary>
		//// <param name="descriptors"></param>
		//// <returns></returns>
		RHI_DescriptorSet* RHI_DescriptorSetManager::GetSet(const DescriptorSet& descriptor_set)
		{
			IS_PROFILE_FUNCTION();

			u64 hash = descriptor_set.Hash_No_Resource;
			u64 hashWithResource = 0;
			{
				IS_PROFILE_SCOPE("Get hashes");
				for (const auto& d : descriptor_set.Bindings)
				{
					if (d.Type != DescriptorType::Unknown)
					{
						//HashCombine(hash, d.GetHash(false));
						HashCombine(hashWithResource, d.GetHash(true));
					}
				}
			}

			// Is there a set which is already in use with the same resources reuse that set.
			// This will also return the correct set if dynamic buffers are being used.
			{
				IS_PROFILE_SCOPE("Return in use set");
				if (auto itr = m_usedSets.find(hash); itr != m_usedSets.end())
				{
					if (auto itrWithResource = itr->second.find(hashWithResource);
						itrWithResource != itr->second.end())
					{
						return itrWithResource->second;
					}
				}
			}

			// No set in use with the same resources, try and find a new set.
			{
				IS_PROFILE_SCOPE("Return in free set");
				if (auto itr = m_freeSets.find(hash); itr != m_freeSets.end())
				{
					if (!itr->second.empty())
					{
						RHI_DescriptorSet* set = itr->second.front();
						itr->second.pop_front();
						m_usedSets[hash][hashWithResource] = set;
						set->Update(descriptor_set);
						return set;
					}
				}
			}

			RHI_DescriptorSet* newSet = nullptr;
			{
				IS_PROFILE_SCOPE("New set");
				newSet = NewArgsTracked(RHI_DescriptorSet, &RenderContext::Instance()
					, descriptor_set
					, RenderContext::Instance().GetDescriptorLayoutManager().GetLayout(descriptor_set));

				m_usedSets[hash][hashWithResource] = newSet;
			}
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

			m_descriptor_sets = pso.Shader->GetDescriptorSets();

			// Reset the hash used for DX12.
			for (DescriptorSet& set : m_descriptor_sets)
			{
				set.DX_Hash = 0;
			}
		}

		bool DescriptorAllocator::WasUniformBufferResized() const
		{
			return m_uniformBufferResized;
		}

		RHI_BufferView DescriptorAllocator::UploadUniform(const void* data, u32 size)
		{
			CreateUniformBufferIfNoExist();

#ifdef DESCRIPTOR_CACHE_UNIFOM_DATA
			u64 hash = 0;
			{
				IS_PROFILE_SCOPE("[DescriptorAllocator::UploadUniform] Hash update data");
#ifdef DESCRIPTOR_CACHE_UNIFOM_DATA_BYTE
				for (size_t i = 0; i < size; ++i)
				{
					const Byte* byteDataPtr = static_cast<const Byte*>(data) + i;
					Byte byteDataValue = *byteDataPtr;
					HashCombine(hash, byteDataValue);
				}
#elif defined(DESCRIPTOR_CACHE_UNIFOM_DATA_4_BYTE)
				ASSERT(size % 4 == 0);
				// Hash the data values of the data wanting to be uploaded.
				for (size_t i = 0; i < size; i += 4)
				{
					const unsigned int* dataPtr = static_cast<const unsigned int*>(data) + i;
					const unsigned int dataValue = *dataPtr;
					HashCombine(hash, dataValue);
				}
#endif // DESCRIPTOR_CACHE_UNIFOM_DATA_BYTE || DESCRIPTOR_CACHE_UNIFOM_DATA_4_BYTE
			}

			// Check has this data already been uploaded to the uniform buffer to the GPU.
			// if so then just return the same location. This reduces the amount being uploaded.
			if (auto iter = m_cachedBufferData.find(hash); iter != m_cachedBufferData.end())
			{
				return iter->second;
			}
#endif //DESCRIPTOR_CACHE_UNIFOM_DATA

			const u32 alignedSize = AlignUp(size, PhysicalDeviceInformation::Instance().MinUniformBufferAlignment);
			if (m_uniformBufferResized
				|| m_uniformBufferOffset + alignedSize > m_uniformBuffer->GetSize())
			{
				m_uniformBufferResized = true;
				m_uniformBufferOffset += m_uniformBufferOffset + alignedSize;
				return {};
			}

			RHI_BufferView view = m_uniformBuffer->Upload(data, static_cast<int>(size), static_cast<int>(m_uniformBufferOffset), PhysicalDeviceInformation::Instance().MinUniformBufferAlignment);
			RenderStats::Instance().FrameUniformBufferSize += view.GetSize();
			m_uniformBufferOffset += view.GetSize();
#ifdef DESCRIPTOR_CACHE_UNIFOM_DATA
			m_cachedBufferData[hash] = view;
#endif // DESCRIPTOR_CACHE_UNIFOM_DATA
			return view;
		}

		void DescriptorAllocator::SetUniform(u32 set, u32 binding, const void* data, u32 size)
		{
			if (!CheckSetAndBindingBounds(set, binding))
			{
				return;
			}

			if (DescriptorSet* descriptorSet = GetDescriptorSet(set))
			{
				if (DescriptorBinding* descriptorBinding = GetDescriptorBinding(descriptorSet, binding, DescriptorType::Unifom_Buffer))
				{
					if (descriptorBinding->Size != size)
					{
						IS_CORE_WARN("[DescriptorAllocator::SetUniform] Size mismatch. Descriptor expects '{0}', provided '{1}'\n Set: {2}, Binding: {3}."
							, descriptorBinding->Size, size, set, binding);
					}
					//if (descriptorBinding->RHI_Buffer_View != bufferView)
					{
						descriptorBinding->RHI_Buffer_View = UploadUniform(data, size);
						HashCombine(descriptorSet->DX_Hash, descriptorBinding->RHI_Buffer_View);
					}
				}
			}
		}

		void DescriptorAllocator::SetUniform(u32 set, u32 binding, RHI_BufferView buffer_view)
		{
			if (!CheckSetAndBindingBounds(set, binding))
			{
				return;
			}

			if (!buffer_view.IsValid())
			{
				IS_CORE_ERROR("[DescriptorAllocator::SetUniform] Trying to set a uniform which buffer view is invalid.");
				return;
			}

			if (DescriptorSet* descriptorSet = GetDescriptorSet(set))
			{
				if (DescriptorBinding* descriptorBinding = GetDescriptorBinding(descriptorSet, binding, DescriptorType::Unifom_Buffer))
				{
					//if (descriptorBinding->RHI_Buffer_View != buffer_view)
					{
						descriptorBinding->RHI_Buffer_View = buffer_view;
						HashCombine(descriptorSet->DX_Hash, descriptorBinding->RHI_Buffer_View);
					}
				}
			}
		}

		void DescriptorAllocator::SetTexture(u32 set, u32 binding, const RHI_Texture* texture)
		{
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				// Apply an offset to our bindings as vulkan shaders are compiled with these offsets 
				// due to the possible binding idx conflict.
				binding += c_VulkanTextureBindingShift;
			}

			if (!CheckSetAndBindingBounds(set, binding))
			{
				return;
			}

			if (DescriptorSet* descriptorSet = GetDescriptorSet(set))
			{
				if (DescriptorBinding* descriptorBinding = GetDescriptorBinding(descriptorSet, binding, DescriptorType::Sampled_Image))
				{
					//if (descriptorBinding->RHI_Texture != texture)
					{
						descriptorBinding->RHI_Texture = texture;
						HashCombine(descriptorSet->DX_Hash, descriptorBinding->RHI_Texture);
					}
				}
			}
		}

		void DescriptorAllocator::SetSampler(u32 set, u32 binding, const RHI_Sampler* sampler)
		{
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				// Apply an offset to our bindings as vulkan shaders are compiled with these offsets 
				// due to the possible binding idx conflict.
				binding += c_VulkanSamplerBindingShift;
			}

			if (!CheckSetAndBindingBounds(set, binding))
			{
				return;
			}

			if (DescriptorSet* descriptorSet = GetDescriptorSet(set))
			{
				if (DescriptorBinding* descriptorBinding = GetDescriptorBinding(descriptorSet, binding, DescriptorType::Sampler))
				{
					if (descriptorBinding->RHI_Sampler != sampler)
					{
						descriptorBinding->RHI_Sampler = sampler;
						HashCombine(descriptorSet->DX_Hash, descriptorBinding->RHI_Sampler);
					}
				}
			}
		}

		std::vector<DescriptorSet> const& DescriptorAllocator::GetAllocatorDescriptorSets() const
		{
			return m_descriptor_sets;
		}

		void DescriptorAllocator::SetRenderContext(RenderContext* context)
		{
			m_context = context;
		}

		bool DescriptorAllocator::GetDescriptorSets(std::vector<RHI_DescriptorSet*>& sets)
		{
			IS_PROFILE_FUNCTION();
			for (const auto& set : m_descriptor_sets)
			{
				IS_PROFILE_SCOPE("Get single set");
				sets.push_back(m_context->GetDescriptorSetManager().GetSet(set));
			}
			return sets.size() > 0 ? true : false;
		}

		std::vector<u32> DescriptorAllocator::GetDynamicOffsets() const
		{
			std::vector<u32> offsets;
			for (const auto& set : m_descriptor_sets)
			{
				for (const auto& binding : set.Bindings)
				{
					if (binding.Type == DescriptorType::Uniform_Buffer_Dynamic) 
					{
						offsets.push_back(static_cast<u32>(binding.RHI_Buffer_View.GetOffset()));
					}
				}
			}
			return offsets;
		}

		void DescriptorAllocator::ClearDescriptors()
		{
			m_descriptor_sets.clear();
		}

		void DescriptorAllocator::Reset()
		{
			ClearDescriptors();

			if (m_uniformBufferResized)
			{
				u64 newSize = m_uniformBufferOffset * 1.8f;
				m_uniformBuffer->Resize(newSize);
			}

			m_uniformBufferOffset = 0;
			m_uniformBufferResized = false;
#ifdef DESCRIPTOR_CACHE_UNIFOM_DATA
			m_cachedBufferData.clear();
#endif // DESCRIPTOR_CACHE_UNIFOM_DATA
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
				m_uniformBuffer = UPtr(Renderer::CreateUniformBuffer(32_MB));
				m_uniformBuffer->SetName("Descriptor_Uniform_Buffer");
			}
		}

		bool DescriptorAllocator::CheckSetAndBindingBounds(u32 set, u32 binding)
		{
			DescriptorSet* descriptor_set = nullptr;
			for (size_t i = 0; i < m_descriptor_sets.size(); ++i)
			{
				if (m_descriptor_sets.at(i).Set == set)
				{
					descriptor_set = &m_descriptor_sets.at(i);
					break;
				}
			}
			if (!descriptor_set)
			{
				//IS_CORE_ERROR("[GPUDescriptorAllocator::CheckSetAndBindingBounds] Set: '{}' is out of range.", set);
				return false;
			}

			bool foundBinding = false;
			for (size_t i = 0; i < descriptor_set->Bindings.size(); ++i)
			{
				if (descriptor_set->Bindings.at(i).Binding == binding)
				{
					foundBinding = true;
					break;
				}
			}

			if (!foundBinding)
			{
				//IS_CORE_ERROR("[DescriptorAllocator::CheckSetAndBindingBounds] Binding: '{0}' is out of range.", binding);
				return false;
			}
			return true;
		}

		DescriptorSet* DescriptorAllocator::GetDescriptorSet(u32 set)
		{
			for (auto& descriptorSet : m_descriptor_sets)
			{
				if (descriptorSet.Set == set)
				{
					return &descriptorSet;
				}
			}
			return nullptr;
		}

		DescriptorBinding* DescriptorAllocator::GetDescriptorBinding(DescriptorSet* set, u32 binding, DescriptorType descriptorType)
		{
			if (set == nullptr)
			{
				return nullptr;
			}


			for (auto& descriptorBinding : set->Bindings)
			{
				if (descriptorBinding.Binding == binding
					&& descriptorBinding.Type == descriptorType)
				{
					return &descriptorBinding;
				}
			}
			return nullptr;
		}
	}
}