#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"

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

					if (!descriptor.BufferView.IsValid())
					{
						continue;
					}

					if (descriptor.Type == DescriptorType::Sampler)
					{

					}
					else if (descriptor.Type == DescriptorType::Combined_Image_Sampler)
					{

					}
					else if (descriptor.Type == DescriptorType::Unifom_Buffer)
					{
						buffer_infos[descriptorWriteIndex].setBuffer( dynamic_cast<RHI_Buffer_Vulkan*>(descriptor.BufferView.GetBuffer())->GetBuffer());
						buffer_infos[descriptorWriteIndex].setOffset(descriptor.BufferView.GetOffset());
						buffer_infos[descriptorWriteIndex].setRange(descriptor.BufferView.GetSize());
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
					m_context->GetDevice().updateDescriptorSets(descriptorWriteIndex, write_descriptor_sets.data(), 0, {});
					m_hash = hash;
				}
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
				u64 hash = 0;
				for (const Descriptor& desc : descriptors)
				{
					HashCombine(hash, desc.GetHash(false));
				}

				++m_size;
				if (auto itr = m_freeDescriptors.find(hash); itr != m_freeDescriptors.end())
				{
					RHI_Descriptor* returnDescriptor = itr->second;
					m_freeDescriptors.erase(hash);
					m_usedDescriptors[hash] = returnDescriptor;

					return returnDescriptor;
				}

				// Create a new descriptor.
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

				m_usedDescriptors[hash] = newDescriptor;
				m_descriptorToHash[newDescriptor] = hash;

				return newDescriptor;
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
				DescriptorPoolPage_Vulkan pool(m_context, 256);
				m_pools.push_back(pool);
			}

			RHI_Descriptor* DescriptorPool_Vulkan::GetDescriptor(const std::vector<Descriptor>& descriptors)
			{
				DescriptorPoolPage_Vulkan* firstAvailablePool = nullptr;
				RHI_Descriptor* reuseDescriptor = nullptr;

				for (DescriptorPoolPage_Vulkan& p : m_pools)
				{
					if (firstAvailablePool == nullptr && !p.IsFull())
					{
						firstAvailablePool = &p;
					}
					
					if (p.HasFreeDescriptor(descriptors))
					{
						reuseDescriptor = p.GetDescriptorSet(descriptors);
						break;
					}
				}

				// Try and reuse a descriptor which has already been allocated,
				// if no free descriptor found, allocate a new descriptor from the first available pool,
				// if no available pool, allocate a new pool and allocate a descriptor.
				if (reuseDescriptor)
				{
					return reuseDescriptor;
				}
				else if (firstAvailablePool)
				{
					return firstAvailablePool->GetDescriptorSet(descriptors);
				}

				AddNewPool();
				return m_pools.back().GetDescriptorSet(descriptors);
			}

			void DescriptorPool_Vulkan::FreeDescriptor(RHI_Descriptor* descriptor)
			{
				for (DescriptorPoolPage_Vulkan& pool : m_pools)
				{
					FreeDescriptor(descriptor);
				}
			}

			void DescriptorPool_Vulkan::Reset()
			{
				for (DescriptorPoolPage_Vulkan& pool : m_pools)
				{
					pool.Reset();
				}
			}

			void DescriptorPool_Vulkan::Destroy()
			{
				for (DescriptorPoolPage_Vulkan& pool : m_pools)
				{
					pool.Release();
				}
				m_pools.resize(0);
			}

			void DescriptorPool_Vulkan::AddNewPool()
			{
				DescriptorPoolPage_Vulkan pool(m_context, 256);
				m_pools.push_back(pool);
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