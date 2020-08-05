#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/VulkanMaterial.h"
#include "Insight/Renderer/Vulkan/VulkanRenderer.h"
#include "Insight/Renderer/Vulkan/Device.h"

#include "Insight/Time/Time.h"

namespace Insight
{
	namespace Render
	{
		VulkanRenderer* VulkanMaterial::s_Renderer;

		void CreateBufferMat(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
		{
			VkBufferCreateInfo createInfo = VulkanInits::BufferInfo();
			createInfo.size = size;
			createInfo.usage = usage;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			ThrowIfFailed(vkCreateBuffer(device->GetDevice(), &createInfo, nullptr, &buffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device->GetDevice(), buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = VulkanInits::MemoryAllocInfo();
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = device->GetMemoryType(memRequirements.memoryTypeBits, properties);

			ThrowIfFailed(vkAllocateMemory(device->GetDevice(), &allocInfo, nullptr, &bufferMemory));
			vkBindBufferMemory(device->GetDevice(), buffer, bufferMemory, 0);
		}

		void alignedFree(void* data)
		{
#if	defined(_MSC_VER) || defined(__MINGW32__)
			_aligned_free(data);
#else
			free(data);
#endif
		}
		VulkanMaterial::VulkanMaterial()
		{
			m_usageCount = 0;
		}

		VulkanMaterial::~VulkanMaterial()
		{
			if (m_uniformBuffers != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformBuffers, nullptr);
			}

			if (m_uniformBuffersMem != VK_NULL_HANDLE)
			{
				vkFreeMemory(s_Renderer->GetDevice(), m_uniformBuffersMem, nullptr);
			}

			for (auto it = m_uniformData.begin(); it != m_uniformData.end(); ++it )
			{
				UnMapBufferMem(it->second);
				vkDestroyBuffer(s_Renderer->GetDevice(), (*it).second.Buffer, nullptr);
				vkFreeMemory(s_Renderer->GetDevice(), (*it).second.BufferMem, nullptr);
			}

			Memory::MemoryManager::DeleteOnFreeList(m_descPool);
		}

		void VulkanMaterial::SetShader(Shader* shader)
		{
			m_shader = static_cast<VulkanShader*>(shader);
		
			auto metaData = m_shader->GetMetaData();
			VkDeviceSize bufferSize = 0;
			for (auto it = metaData[0].UniformBlocks.begin(); it != metaData[0].UniformBlocks.end(); ++it)
			{
				bufferSize += (*it).Size;
			}

			if (m_uniformBuffers != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformBuffers, nullptr);
				vkFreeMemory(s_Renderer->GetDevice(), m_uniformBuffersMem, nullptr);
			}
			if (bufferSize != 0)
			{
				CreateBufferMat(s_Renderer->GetDeviceWrapper(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers, m_uniformBuffersMem);
			}

			m_descPool = NEW_ON_HEAP(DescriptorPool, s_Renderer->GetDeviceWrapper(), metaData,
				VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
			m_updatedDesc = false;
			m_descPool->FreeDescriptorSets();
			m_descSet = m_descPool->AllocDescriptorSet(m_shader->GetDescLayout());
			SetUniforms();
		}

		Shader* VulkanMaterial::GetShader()
		{
			return m_shader;
		}

		void VulkanMaterial::SetUniforms()
		{
			if (m_isDirty)
			{

			}

			std::vector<UniformData*> uniformData;
			std::vector<SamplerData*> samplerData;
			for (auto it = m_uniformData.begin(); it != m_uniformData.end(); ++it)
			{
				uniformData.push_back(&(*it).second);
			}
			for (auto it = m_samplerData.begin(); it != m_samplerData.end(); ++it)
			{
				samplerData.push_back(&(*it).second);
			}
			m_descSet->UpdateDescriptorSet(s_Renderer->GetDevice(), uniformData, samplerData);
		}

		MVPUniformBuffer m_mvp;
		void VulkanMaterial::UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model)
		{
			UniformData data;

			if (m_uniformData.find("MVP") == m_uniformData.end())
			{
				data.TypeSize = sizeof(MVPUniformBuffer);
				data.Size = data.TypeSize;
				data.Binding = 0;
				data.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				CreateUniformBufferMem(data);
				MapBufferMem(data);
				m_uniformData["MVP"] = data;
			}
			data = m_uniformData["MVP"];

			m_mvp.u_view = view;
			m_mvp.u_proj = proj;
			m_mvp.u_proj[1][1] *= -1;

			memcpy(data.DataMapped, &m_mvp, sizeof(MVPUniformBuffer));
		}

		void VulkanMaterial::UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding)
		{
			UniformData data;

			if (m_uniformData.find(key) == m_uniformData.end())
			{
				data.Size = size;
				data.Binding = binding;
				data.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				CreateUniformBufferMem(data);
				m_uniformData[key] = data;
			}
			data = m_uniformData[key];

			MapBufferMem(data);
			memcpy(data.DataMapped, uniformData, data.Size);
			UnMapBufferMem(data);
		}

		void VulkanMaterial::UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding)
		{
			SamplerData data;
			data.ImageView = static_cast<VkImageView*>(imageView);
			data.Sampler = static_cast<VkSampler*>(sampler);
			data.Binding = binding;

			m_samplerData[key] = data;
		}

		void VulkanMaterial::IncermentUsageCount()
		{
			++m_usageCount;

		}

		void VulkanMaterial::DecermentUsageCount()
		{
			--m_usageCount;
		}

		void VulkanMaterial::Resize()
		{
			m_descPool->FreeDescriptorSets();
			m_descSet = m_descPool->AllocDescriptorSet(m_shader->GetDescLayout());
		}

		void VulkanMaterial::Bind(CommandBuffer* commandBuffers, int drawIndex)
		{
			m_shader->Bind(commandBuffers);

			// One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
			uint32_t dynamicOffset = drawIndex * static_cast<uint32_t>(m_dynamicOffset);

			if (m_dynamicOffset > 0)
			{
				vkCmdBindDescriptorSets(commandBuffers->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_shader->GetPipelineLayout(), 0, 1, m_descSet->GetSet(), 1, &dynamicOffset);
			}
			else
			{
				vkCmdBindDescriptorSets(commandBuffers->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_shader->GetPipelineLayout(), 0, 1, m_descSet->GetSet(), 0, nullptr);
			}
		}

		void VulkanMaterial::DestroyUniformBuffers()
		{
			//for (auto it = m_uniformBuffers.begin(); it != m_uniformBuffers.end(); ++it)
			{
				vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformBuffers, nullptr);
			}

			//for (auto it = m_uniformBuffersMem.begin(); it != m_uniformBuffersMem.end(); ++it)
			{
				vkFreeMemory(s_Renderer->GetDevice(), m_uniformBuffersMem, nullptr);
			}
		}

		void VulkanMaterial::CreateUniformBufferMem(UniformData& uniformData)
		{
			CreateBufferMat(s_Renderer->GetDeviceWrapper(), uniformData.Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformData.Buffer, uniformData.BufferMem);
		}

		void VulkanMaterial::MapBufferMem(UniformData& uniformData)
		{
			vkMapMemory(s_Renderer->GetDevice(), uniformData.BufferMem, 0, uniformData.Size, 0, &uniformData.DataMapped);
		}

		void VulkanMaterial::UnMapBufferMem(UniformData& uniformData)
		{
			vkUnmapMemory(s_Renderer->GetDevice(), uniformData.BufferMem);
		}
	}
}
#endif