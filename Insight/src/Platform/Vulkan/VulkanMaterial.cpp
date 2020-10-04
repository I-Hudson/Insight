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
#include "Insight/Renderer/Buffer.h"

#define BATCH_RENDERING false
#define USE_MEMCPY false

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
		: m_vBatchBuffer(nullptr)
		, m_iBatchBuffer(nullptr)
	{
		m_usageCount = 0;

		// Calculate required alignment based on minimum device offset alignment
		size_t minUboAlignment = s_Renderer->GetDeviceWrapper()->GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
		int dynamicAlignment = sizeof(glm::mat4);
		if (minUboAlignment > 0)
		{
			dynamicAlignment = static_cast<int>((dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1));
		}
		m_minDynamicOffset = dynamicAlignment;
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

		if (m_uniformObjectsData.Buffer != VK_NULL_HANDLE)
		{
			DELETE_ARR_ON_HEAP(m_uniformObjectsData.Data, m_uniformObjectsData.Size);
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
		bool createDesc = false;
		for (size_t i = 0; i < metaData.size(); ++i)
		{
			for (auto it = metaData[i].UniformBlocks.begin(); it != metaData[i].UniformBlocks.end(); ++it)
			{
				bufferSize += (*it).Size;
				createDesc = true;
			
				if (!m_hasDynamicUniform)
				{
					m_hasDynamicUniform = Insight::Render::ShaderModuleBase::GetShaderDescriptorType((*it).Type) == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				}
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

		if (createDesc)
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
		using namespace Insight;

		int index = 0;
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			UniformObjectsData data = *it;//m_uniformObjectsData.Positions[0];
			glm::mat4 model = ((TransformComponent*)data.Data)->GetTransform();
			((glm::mat4*)m_uniformObjectsData.Data)[index++] = model;
			//((void*)((int*)m_uniformObjectsData.DataMapped + data.Offset), &model, data.Size);
		}
		memcpy_s(m_uniformObjectsData.DataMapped, m_uniformObjectsData.Size, m_uniformObjectsData.Data, m_uniformObjectsData.Size);

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
			data.Size = static_cast<int>(size);
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

	MaterialRenderData VulkanMaterial::IncrementUsageCount(const MeshComponent* meshComponent)
	{
		++m_usageCount;
		m_isDirty = true;

		VKMaterialRenderData materialRendererData;

		UniformObjectsData data;
		data.Data = meshComponent->GetEntity()->GetComponent<TransformComponent>();
		data.TypeSize = sizeof(glm::mat4);
		data.Size = data.TypeSize;
		data.Binding = 1;
		data.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		data.MeshCompoentPtr = const_cast<MeshComponent*>(meshComponent);

		m_uniformObjectsData.Positions.push_back(data);

		if (m_uniformObjectsData.Size > 0)
		{
			//Recreate objects dyanimc buffer
			UnMapBufferMem(m_uniformObjectsData);
			vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformObjectsData.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_uniformObjectsData.BufferMem, nullptr);
			DELETE_ARR_ON_HEAP(m_uniformObjectsData.Data, m_uniformObjectsData.Size);

			m_uniformObjectsData.Buffer = VK_NULL_HANDLE;
		}

		m_uniformObjectsData.TypeSize = sizeof(glm::mat4);
		m_uniformObjectsData.Binding = 1;
		m_uniformObjectsData.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		m_uniformObjectsData.Size = static_cast<int>(m_uniformObjectsData.Positions.size() * sizeof(glm::mat4));
		if (m_uniformObjectsData.Size < m_minDynamicOffset && m_uniformObjectsData.Positions.size() > 1)
		{
			m_uniformObjectsData.Size = m_minDynamicOffset;
		}

		if (m_uniformObjectsData.Size > 0)
		{
			CreateUniformBufferMem(m_uniformObjectsData);
			MapBufferMem(m_uniformObjectsData);

			m_uniformObjectsData.Data = NEW_ARR_ON_HEAP(glm::mat4, m_uniformObjectsData.Positions.size());

			for (size_t i = 0; i < m_uniformObjectsData.Positions.size(); ++i)
			{
				m_uniformObjectsData.Positions[i].Offset = static_cast<int>(i * m_uniformObjectsData.TypeSize);
			}

		materialRendererData.State = MaterialRenderDataState::Valid;
		materialRendererData.PositionDynamicUniformOffset = (int)m_uniformObjectsData.Positions.size() - 1;
		}
		else
		{
			materialRendererData.State = MaterialRenderDataState::Invalid;
		}

		return materialRendererData;
	}

	void VulkanMaterial::DecrementUsageCount(const MeshComponent* meshComponent)
	{
		--m_usageCount;
		m_isDirty = true;

		UniformObjectsData data;
		TransformComponent* meshTransform = meshComponent->GetEntity()->GetComponent<TransformComponent>();
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			if (((TransformComponent*)it->Data) == meshTransform)
			{
				m_uniformObjectsData.Positions.erase(it);
				break;
			}
		}

		if (m_uniformObjectsData.Size > 0)
		{
			//Recreate objects dyanimc buffer
			UnMapBufferMem(m_uniformObjectsData);
			vkDestroyBuffer(s_Renderer->GetDevice(), m_uniformObjectsData.Buffer, nullptr);
			vkFreeMemory(s_Renderer->GetDevice(), m_uniformObjectsData.BufferMem, nullptr);
			DELETE_ARR_ON_HEAP(m_uniformObjectsData.Data, m_uniformObjectsData.Size);

			m_uniformObjectsData.Buffer = VK_NULL_HANDLE;
		}

		m_uniformObjectsData.TypeSize = sizeof(glm::mat4);
		m_uniformObjectsData.Binding = 1;
		m_uniformObjectsData.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		m_uniformObjectsData.Size = static_cast<int>(m_uniformObjectsData.Positions.size() * sizeof(glm::mat4));
		if (m_uniformObjectsData.Size < m_minDynamicOffset && m_uniformObjectsData.Positions.size() > 1)
		{
			m_uniformObjectsData.Size = m_minDynamicOffset;
		}

		if (m_uniformObjectsData.Size > 0)
		{
			CreateUniformBufferMem(m_uniformObjectsData);
			MapBufferMem(m_uniformObjectsData);

			m_uniformObjectsData.Data = NEW_ARR_ON_HEAP(glm::mat4, m_uniformObjectsData.Positions.size());
			
			for (size_t i = 0; i < m_uniformObjectsData.Positions.size(); ++i)
			{
				m_uniformObjectsData.Positions[i].Offset = static_cast<int>(i * 64);
			}
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

	void VulkanMaterial::Bind(CommandBuffer* commandBuffers, MeshComponent* meshBeingDrawn)
	{
		IS_PROFILE_FUNCTION();

		m_shader->Bind(commandBuffers);

		int index = 0;
		bool shouldBreak = false;
		for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
		{
			if ((*it).MeshCompoentPtr == meshBeingDrawn)
			{
				index = static_cast<int>(it - m_uniformObjectsData.Positions.begin());
				shouldBreak = true;
				break;
			}
		
			if (shouldBreak)
			{
				break;
			}
		}

		// One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
		//MaterialRenderData materialRendererData = meshBeingDrawn->GetMaterialRendererData();
		//VKMaterialRenderData vkMaterialRendererData = (VKMaterialRenderData)materialRendererData; // Didn't work
		uint32_t dynamicOffset = index * static_cast<uint32_t>(64);

		if (m_descSet == nullptr)
		{
			return;
		}

		if (m_hasDynamicUniform)
		{
			vkCmdBindDescriptorSets(commandBuffers->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_shader->GetPipelineLayout(), 0, 1, m_descSet->GetSet(), 1, &dynamicOffset);
		}
		else
		{
			vkCmdBindDescriptorSets(commandBuffers->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_shader->GetPipelineLayout(), 0, 1, m_descSet->GetSet(), 0, nullptr);
		}

		if (BATCH_RENDERING)
		{
			IS_PROFILE_SCOPE("Batch render buffers");

			DELETE_ON_HEAP(m_vBatchBuffer);
			DELETE_ON_HEAP(m_iBatchBuffer);

			// 740.11 ms
			if (m_vBatchBuffer == nullptr && m_iBatchBuffer == nullptr && !USE_MEMCPY)
			{
				std::vector<Vertex> vertices;
				std::vector<unsigned int> indices;

				{
					IS_PROFILE_SCOPE("Batch Vertices");
					for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
					{
						auto meshVertices = ((TransformComponent*)(*it).Data)->GetComponent<MeshComponent>()->GetVertices();
						vertices.insert(vertices.end(), meshVertices.begin(), meshVertices.end());
					}
				}

				{
					IS_PROFILE_SCOPE("Batch Indices");
					for (auto it = m_uniformObjectsData.Positions.begin(); it != m_uniformObjectsData.Positions.end(); ++it)
					{
						auto meshIndices = ((TransformComponent*)(*it).Data)->GetComponent<MeshComponent>()->GetIndices();
						indices.insert(indices.end(), meshIndices.begin(), meshIndices.end());
					}
				}
			
				m_vBatchBuffer = Insight::Render::VertexBuffer::Create(vertices);
				m_iBatchBuffer = Insight::Render::IndexBuffer::Create(indices);
			}
			else
			{

			}
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