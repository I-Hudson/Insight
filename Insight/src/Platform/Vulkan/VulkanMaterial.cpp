#include "ispch.h"
#ifdef IS_VULKAN
#include "VulkanMaterial.h"
#include "VulkanRenderer.h"
#include "Device.h"

#include "Insight/Component/MeshComponent.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/TransformComponent.h"
#include "Insight/Time/Time.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Platform
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

		// Calculate required alignment based on minimum device offset alignment
		size_t minUboAlignment = s_Renderer->GetDeviceWrapper()->GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
		int dynamicAlignment = sizeof(glm::mat4);
		if (minUboAlignment > 0)
		{
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		m_dynamicOffset = dynamicAlignment;
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

		for (auto it = m_uniformData.begin(); it != m_uniformData.end(); ++it)
		{
			UnMapBufferMem(it->second);
			vkDestroyBuffer(s_Renderer->GetDevice(), (*it).second.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), (*it).second.BufferMem, nullptr);
		}

		if (m_uniformObjectsData.Buffer != nullptr)
		{
			UnMapBufferMem(m_uniformObjectsData);
			vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformObjectsData.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_uniformObjectsData.BufferMem, nullptr);
		}

		for (auto it = m_uniformDynamicData.begin(); it != m_uniformDynamicData.end(); ++it)
		{
			UnMapBufferMem(it->second);
			vkDestroyBuffer(s_Renderer->GetDevice(), (*it).second.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), (*it).second.BufferMem, nullptr);
		}

		DELETE_ON_HEAP(m_descPool);
	}

	void VulkanMaterial::SetShader(Insight::Render::Shader* shader)
	{
		m_shader = static_cast<VulkanShader*>(shader);

		auto metaData = m_shader->GetMetaData();
		VkDeviceSize bufferSize = 0;
		for (size_t i = 0; i < metaData.size(); ++i)
		{
			for (auto it = metaData[i].UniformBlocks.begin(); it != metaData[i].UniformBlocks.end(); ++it)
			{
				bufferSize += (*it).Size;
			}
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

		if (bufferSize > 0)
		{
			m_descPool = NEW_ON_HEAP(DescriptorPool, s_Renderer->GetDeviceWrapper(), metaData,
				VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
			m_updatedDesc = false;
			m_descPool->FreeDescriptorSets();
			m_descSet = m_descPool->AllocDescriptorSet(m_shader->GetDescLayout());
			SetUniforms();
		}
		else
		{
			m_descPool = nullptr;
			m_descSet = nullptr;
		}
	}

	Insight::Render::Shader* VulkanMaterial::GetShader()
	{
		return m_shader;
	}

	void VulkanMaterial::SetUniforms()
	{
		if (m_descSet == nullptr)
		{
			return;
		}

		std::vector<UniformData*> uniformData;
		std::vector<SamplerData*> samplerData;
		for (auto it = m_uniformData.begin(); it != m_uniformData.end(); ++it)
		{
			uniformData.push_back(&(*it).second);
		}

		if (m_uniformObjectsData.Size != 0)
		{
			uniformData.push_back(&(m_uniformObjectsData));
		}

		for (auto it = m_uniformDynamicData.begin(); it != m_uniformDynamicData.end(); ++it)
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

	void VulkanMaterial::UpdateObjectsUniforms()
	{
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			UniformObjectsData data = *it;
			memcpy((void*)((int*)m_uniformObjectsData.DataMapped + data.Offset), &data.Data, data.Size);
		}

		VkMappedMemoryRange mappedMemoryRange{};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

		// Flush to make changes visible to the host
		VkMappedMemoryRange memoryRange = mappedMemoryRange;
		memoryRange.memory = m_uniformObjectsData.BufferMem;
		memoryRange.size = m_uniformObjectsData.Size;
		vkFlushMappedMemoryRanges(s_Renderer->GetDevice(), 1, &memoryRange);
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
		data.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		m_samplerData[key] = data;
	}

	void VulkanMaterial::IncrementUsageCount(const MeshComponent* meshComponent)
	{
		++m_usageCount;
		m_isDirty = true;

		std::vector<std::unordered_map<std::string, UniformDyanmicData>::iterator> removeIts;

		for (auto it = m_uniformDynamicData.begin(); it != m_uniformDynamicData.end(); ++it)
		{
			UnMapBufferMem(it->second);
			vkDestroyBuffer(s_Renderer->GetDevice(), (*it).second.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), (*it).second.BufferMem, nullptr);
			removeIts.push_back(it);
		}

		for (size_t i = 0; i < removeIts.size(); ++i)
		{
			m_uniformDynamicData.erase(removeIts[i]);
		}

		UniformObjectsData data;
		bool foundData = false;
		glm::mat4 meshTransform = meshComponent->GetEntity()->GetComponent<TransformComponent>()->GetTransform();
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			if (it->Data == meshTransform)
			{
				it->Owners.push_back(const_cast<MeshComponent*>(meshComponent));
				foundData = true;
				break;
			}
		}

		if (!foundData)
		{
			data.Data = meshTransform;
			data.TypeSize = sizeof(glm::mat4);
			data.Size = data.TypeSize;
			data.Binding = 1;
			data.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			data.Owners.push_back(const_cast<MeshComponent*>(meshComponent));


			m_uniformObjectsData.Positions.push_back(data);
		}

		if (m_uniformObjectsData.Size > 0)
		{
			//Recreate objects dyanimc buffer
			UnMapBufferMem(m_uniformObjectsData);
			vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformObjectsData.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_uniformObjectsData.BufferMem, nullptr);
		}

		m_uniformObjectsData.TypeSize = sizeof(glm::mat4);
		m_uniformObjectsData.Binding = 1;
		m_uniformObjectsData.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		m_uniformObjectsData.Size = m_uniformObjectsData.Positions.size() * sizeof(glm::mat4);
		CreateUniformBufferMem(m_uniformObjectsData);
		MapBufferMem(m_uniformObjectsData);

		for (size_t i = 0; i < m_uniformObjectsData.Positions.size(); ++i)
		{
			m_uniformObjectsData.Positions[i].Offset = i * 64;
		}
	}

	void VulkanMaterial::DecrementUsageCount(const MeshComponent* meshComponent)
	{
		--m_usageCount;
		m_isDirty = true;

		UniformObjectsData data;
		glm::mat4 meshTransform = meshComponent->GetEntity()->GetComponent<TransformComponent>()->GetTransform();
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			if (it->Data == meshTransform)
			{
				it->Owners.erase(std::find(it->Owners.begin(), it->Owners.end(), meshComponent));
				break;
			}
		}

		if (m_uniformObjectsData.Size > 0)
		{
			//Recreate objects dyanimc buffer
			UnMapBufferMem(m_uniformObjectsData);
			vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformObjectsData.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_uniformObjectsData.BufferMem, nullptr);
		}

		m_uniformObjectsData.TypeSize = sizeof(glm::mat4);
		m_uniformObjectsData.Binding = 1;
		m_uniformObjectsData.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		m_uniformObjectsData.Size = static_cast<int>(m_uniformObjectsData.Positions.size() * sizeof(glm::mat4));
		CreateUniformBufferMem(m_uniformObjectsData);
		MapBufferMem(m_uniformObjectsData);

		for (size_t i = 0; i < m_uniformObjectsData.Positions.size(); ++i)
		{
			m_uniformObjectsData.Positions[i].Offset = static_cast<int>(i * 64);
		}
	}

	void VulkanMaterial::Resize()
	{
		if (m_descPool != nullptr)
		{
			m_descPool->FreeDescriptorSets();
			m_descSet = m_descPool->AllocDescriptorSet(m_shader->GetDescLayout());
		}
	}

	void VulkanMaterial::Bind(CommandBuffer* commandBuffers, const MeshComponent* meshBeingDrawn)
	{
		IS_PROFILE_FUNCTION();

		m_shader->Bind(commandBuffers);

		int index = 0;
		bool shouldBreak = false;
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			for (auto itt = it->Owners.begin(); itt != it->Owners.end(); ++itt)
			{
				if (*itt == meshBeingDrawn)
				{
					index = static_cast<int>(it - m_uniformObjectsData.Positions.begin());
					shouldBreak = true;
					break;
				}
			}

			if (shouldBreak)
			{
				break;
			}
		}

		// One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
		uint32_t dynamicOffset = index * static_cast<uint32_t>(m_dynamicOffset);

		if (m_descSet == nullptr)
		{
			return;
		}

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
#endif