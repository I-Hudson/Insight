#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			/// <summary>
			/// RHI_DescriptorLayout_Vulkan
			/// </summary>
			void RHI_DescriptorLayout_Vulkan::Release()
			{
				if (m_layout)
				{
					m_context->GetDevice().destroyDescriptorSetLayout(m_layout);
					m_layout = nullptr;
				}
			}

			bool RHI_DescriptorLayout_Vulkan::ValidResouce()
			{
				return m_layout;
			}

			void RHI_DescriptorLayout_Vulkan::SetName(std::wstring name)
			{
			}

			void RHI_DescriptorLayout_Vulkan::Create(RenderContext* context, int set, std::vector<Descriptor> descriptors)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);

				vk::DescriptorSetLayoutCreateInfo setCreateInfo = {};
				std::vector<vk::DescriptorSetLayoutBinding> bindingCreateInfos = {};
				std::vector<vk::DescriptorBindingFlagsEXT> bindingEXTFlags = {};

				for (const Descriptor& desc : descriptors)
				{
					vk::DescriptorSetLayoutBinding bindingInfo = {};
					bindingInfo.setBinding(desc.Binding);
					bindingInfo.setDescriptorCount(1);
					bindingInfo.setDescriptorType(DescriptorTypeToVulkan(desc.Type));
					bindingInfo.setStageFlags(ShaderStageFlagsToVulkan(desc.Stage));

					bindingCreateInfos.push_back(bindingInfo);
					if (m_context->HasExtension(DeviceExtension::BindlessDescriptors))
					{
						bindingEXTFlags.push_back(vk::DescriptorBindingFlagBitsEXT::ePartiallyBound);
					}
				}

				vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT setEXT = {};
				if (m_context->HasExtension(DeviceExtension::BindlessDescriptors))
				{
					setEXT.setBindingFlags(bindingEXTFlags);
					setCreateInfo.setPNext(&setEXT);
				}
				setCreateInfo.setBindings(bindingCreateInfos);
				m_layout = m_context->GetDevice().createDescriptorSetLayout(setCreateInfo);
			}



			/// <summary>
			/// RHI_Descriptor_Vulkan
			/// </summary>
			/// <param name="descriptors"></param>
			void RHI_Descriptor_Vulkan::Update(const std::vector<Descriptor>& descriptors)
			{
				constexpr u8 descriptors_max = 32;
				std::array<vk::DescriptorImageInfo, descriptors_max> image_infos;
				std::array<vk::DescriptorBufferInfo, descriptors_max> buffer_infos;
				std::array<vk::WriteDescriptorSet, descriptors_max> write_descriptor_sets;
				int descriptorWriteIndex = 0;

				u64 hash = 0;
				for (const Descriptor& descriptor : descriptors)
				{
					HashCombine(hash, descriptor.GetHash(true));

					if (!descriptor.BufferView.IsValid() && !descriptor.Texture)
					{
						continue;
					}

					if (descriptor.Type == DescriptorType::Sampler)
					{

					}
					else if (descriptor.Type == DescriptorType::Combined_Image_Sampler)
					{
						const RHI_Texture_Vulkan* textureVulkan = static_cast<RHI_Texture_Vulkan*>(descriptor.Texture);

						static vk::Sampler sampler;
						if (!sampler)
						{
							vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo(
								{},
								vk::Filter::eLinear,
								vk::Filter::eLinear,
								vk::SamplerMipmapMode::eLinear,
								vk::SamplerAddressMode::eMirroredRepeat,
								vk::SamplerAddressMode::eMirroredRepeat,
								vk::SamplerAddressMode::eMirroredRepeat,
								0.0f, 
								false,
								1.0f,
								false,
								vk::CompareOp::eNever,
								0.0f,
								0.0f,
								vk::BorderColor::eFloatOpaqueWhite);
							sampler = m_context->GetDevice().createSampler(samplerCreateInfo);
						}

						vk::DescriptorImageInfo& imageInfo = image_infos[descriptorWriteIndex];
						imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
						imageInfo.setImageView(textureVulkan->GetImageView());
						imageInfo.setSampler(sampler);
					}
					else if (descriptor.Type == DescriptorType::Unifom_Buffer)
					{
						vk::DescriptorBufferInfo& bufferInfo = buffer_infos[descriptorWriteIndex];
						bufferInfo.setBuffer( dynamic_cast<RHI_Buffer_Vulkan*>(descriptor.BufferView.GetBuffer())->GetBuffer());
						bufferInfo.setOffset(descriptor.BufferView.GetOffset());
						bufferInfo.setRange(descriptor.BufferView.GetSize());
					}

					write_descriptor_sets[descriptorWriteIndex].setDstSet(m_set);
					write_descriptor_sets[descriptorWriteIndex].setDstBinding(descriptor.Binding);
					write_descriptor_sets[descriptorWriteIndex].setDstArrayElement(0);
					write_descriptor_sets[descriptorWriteIndex].setDescriptorCount(1);
					write_descriptor_sets[descriptorWriteIndex].setDescriptorType(DescriptorTypeToVulkan(descriptor.Type));
					write_descriptor_sets[descriptorWriteIndex].setDescriptorCount(1);
					write_descriptor_sets[descriptorWriteIndex].setPImageInfo(&image_infos[descriptorWriteIndex]);
					write_descriptor_sets[descriptorWriteIndex].setPBufferInfo(&buffer_infos[descriptorWriteIndex]);
					write_descriptor_sets[descriptorWriteIndex].setPTexelBufferView(nullptr);

					++descriptorWriteIndex;
				}

				if (hash != m_hash)
				{
					IS_PROFILE_SCOPE("updateDescriptorSets");
					m_context->GetDevice().updateDescriptorSets(descriptorWriteIndex, write_descriptor_sets.data(), 0, {});
					m_hash = hash;
				}
			}

			u64 RHI_Descriptor_Vulkan::GetHash(bool includeResouce)
			{
				return includeResouce ? m_hashWithResouce : m_hash;
			}

			void RHI_Descriptor_Vulkan::Release()
			{
				if (m_set)
				{
					//m_context->GetDevice().freeDescriptorSets(m_pool->GetPool(), { m_set });
					m_set = nullptr;
				}
			}

			bool RHI_Descriptor_Vulkan::ValidResouce()
			{
				return m_set;
			}

			void RHI_Descriptor_Vulkan::SetName(std::wstring name)
			{
			}


			/// <summary>
			/// DescriptorPoolPage_Vulkan
			/// </summary>
			DescriptorPoolPage_Vulkan::DescriptorPoolPage_Vulkan()
			{
			}

			DescriptorPoolPage_Vulkan::DescriptorPoolPage_Vulkan(RenderContext_Vulkan* context, int capacity)
			{
				m_context = context;
				m_capacity = capacity;

				std::vector<vk::DescriptorPoolSize> poolSizes = {};
				poolSizes.push_back(
					vk::DescriptorPoolSize
					{
						vk::DescriptorType::eUniformBuffer,
						(u32)capacity
					});
				poolSizes.push_back(
					vk::DescriptorPoolSize
					{
						vk::DescriptorType::eSampler,
						(u32)capacity
					});
				poolSizes.push_back(
					vk::DescriptorPoolSize
					{
						vk::DescriptorType::eCombinedImageSampler,
						(u32)capacity
					});

				vk::DescriptorPoolCreateInfo createInfo = {};
				createInfo.setPoolSizes(poolSizes);
				createInfo.setMaxSets(capacity);
				m_descriptorPool = m_context->GetDevice().createDescriptorPool(createInfo);
			}

			RHI_Descriptor* DescriptorPoolPage_Vulkan::GetDescriptorSet(const std::vector<Descriptor>& descriptors)
			{
				u64 hashWith = 0;
				u64 hashWithResouce = 0;
				for (const Descriptor& desc : descriptors)
				{
					HashCombine(hashWith, desc.GetHash(false));
					HashCombine(hashWithResouce, desc.GetHash(true));
				}


				// If descriptor with the same resouce is already in use, reuse it.
				if (auto itr = m_usedDescriptors.find(hashWithResouce); itr != m_usedDescriptors.end())
				{
					return itr->second;
				}

				// Do we have a free descriptor which we can use?
				if (m_freeDescriptors.size() > 0)
				{
					const u64 hash = m_freeDescriptors.begin()->first;
					RHI_Descriptor* returnDescriptor = m_freeDescriptors.begin()->second;
					m_freeDescriptors.erase(hash);
					m_usedDescriptors[hash] = returnDescriptor;

					return returnDescriptor;
				}

				++m_size;
				// No in use decriptor found and no free one to resue so make a new descriptor.
				RHI_Descriptor_Vulkan* newDescriptor = dynamic_cast<RHI_Descriptor_Vulkan*>(RHI_Descriptor::New());
				newDescriptor->m_context = m_context;
				newDescriptor->m_pool = this;

				RHI_DescriptorLayout_Vulkan* layout = dynamic_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(descriptors[0].Set, descriptors));
				std::array<vk::DescriptorSetLayout, 1> layouts = { layout->GetLayout()};

				vk::DescriptorSetAllocateInfo createInfo = { };
				createInfo.setDescriptorPool(m_descriptorPool);
				createInfo.setDescriptorSetCount(1);
				createInfo.setPSetLayouts(layouts.data());
				newDescriptor->m_set = m_context->GetDevice().allocateDescriptorSets(createInfo)[0];

				m_usedDescriptors[hashWithResouce] = newDescriptor;
				m_descriptorToHash[newDescriptor] = hashWithResouce;

				return newDescriptor;
			}

			bool DescriptorPoolPage_Vulkan::HasAllocatedDescriptor(const std::vector<Descriptor>& descriptors)
			{
				u64 hash = 0;
				for (const Descriptor& desc : descriptors)
				{
					HashCombine(hash, desc.GetHash(true));
				}
				return m_usedDescriptors.find(hash) != m_usedDescriptors.end();
			}

			bool DescriptorPoolPage_Vulkan::HasFreeDescriptor(const std::vector<Descriptor>& descriptors)
			{
				u64 hash = 0;
				for (const Descriptor& desc : descriptors)
				{
					HashCombine(hash, desc.GetHash(false));
				}
				return m_freeDescriptors.find(hash) != m_freeDescriptors.end();
			}

			void DescriptorPoolPage_Vulkan::FreeDescriptor(RHI_Descriptor* descriptor)
			{
				const u64 hash = m_descriptorToHash[descriptor];
				m_usedDescriptors.erase(hash);
				m_freeDescriptors[hash] = descriptor;
			}

			void DescriptorPoolPage_Vulkan::Reset()
			{
				for (auto& pair : m_usedDescriptors)
				{
					m_freeDescriptors[pair.first] = pair.second;
				}
				m_usedDescriptors.clear();
			}

			void DescriptorPoolPage_Vulkan::Release()
			{
				if (m_descriptorPool)
				{
					for (auto& pair : m_usedDescriptors)
					{
						pair.second->Release();
						DeleteTracked(pair.second);
					}
					for (auto& pair : m_freeDescriptors)
					{
						pair.second->Release();
						DeleteTracked(pair.second);
					}

					m_context->GetDevice().destroyDescriptorPool(m_descriptorPool);
					m_descriptorPool = nullptr;
				}
			}


			/// <summary>
			/// DescriptorPool_Vulkan
			/// </summary>
			/// <param name="context"></param>
			void DescriptorPool_Vulkan::Create(RenderContext_Vulkan* context)
			{
				if (m_pools.size() > 0)
				{
					return;
				}

				m_context = context;
			}

			RHI_Descriptor* DescriptorPool_Vulkan::GetDescriptor(const std::vector<Descriptor>& descriptors)
			{
				u64 descriptorHash = 0;
				for (const Descriptor& desc : descriptors)
				{
					HashCombine(descriptorHash, desc.GetHash(false));
				}

				auto itr = m_pools.find(descriptorHash);
				if (itr != m_pools.end())
				{
					return itr->second.GetDescriptorSet(descriptors);
				}

				AddNewPool(descriptorHash);
				return m_pools.find(descriptorHash)->second.GetDescriptorSet(descriptors);
			}

			void DescriptorPool_Vulkan::FreeDescriptor(RHI_Descriptor* descriptor)
			{
				auto itr = m_pools.find(descriptor->GetHash(false));
				assert(itr != m_pools.end());
				itr->second.FreeDescriptor(descriptor);
			}

			void DescriptorPool_Vulkan::Reset()
			{
				for (auto& pair : m_pools)
				{
					pair.second.Reset();
				}
			}

			void DescriptorPool_Vulkan::Destroy()
			{
				for (auto& pair : m_pools)
				{
					pair.second.Release();
				}
				m_pools.clear();
			}

			void DescriptorPool_Vulkan::AddNewPool(u64 hash)
			{
				DescriptorPoolPage_Vulkan pool(m_context, 256);
				m_pools[hash] = pool;
			}

			/// <summary>
			/// DescriptorAllocator_Vulkan
			/// </summary>
			/// <param name="pso"></param>
			void DescriptorAllocator_Vulkan::SetRenderContext(RenderContext* context)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);
				m_pool.Create(m_context);
			}

			bool DescriptorAllocator_Vulkan::GetDescriptors(std::vector<RHI_Descriptor*>& descriptors)
			{
				for (const auto& pair : m_descriptors)
				{
					const int setIndex = pair.first;
					RHI_Descriptor* descriptor = m_pool.GetDescriptor(pair.second);
					descriptor->Update(pair.second);
					descriptors.push_back(descriptor);
				}

				return true;
			}

			void DescriptorAllocator_Vulkan::Reset()
			{
				m_pool.Reset();
			}

			void DescriptorAllocator_Vulkan::Destroy()
			{
				m_pool.Destroy();
			}
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)