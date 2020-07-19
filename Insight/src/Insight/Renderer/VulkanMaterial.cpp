#include "ispch.h"
#include "VulkanMaterial.h"

#include "Insight/Renderer/VulkanRenderer.h"
#include "Insight/Memory/MemoryManager.h"

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

		VulkanMaterial::VulkanMaterial()
		{
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
				vkDestroyBuffer(s_Renderer->GetDevice(), (*it).second.Buffer, nullptr);
				vkFreeMemory(s_Renderer->GetDevice(), (*it).second.BufferMem, nullptr);
			}

			Memory::MemoryManager::DeleteOnFreeList(m_descPool);
		}

		void VulkanMaterial::SetShader(Shader* shader)
		{
			m_shader = shader;
		
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
			CreateBufferMat(s_Renderer->GetDeviceWrapper(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers, m_uniformBuffersMem);
		
			m_descPool = Memory::MemoryManager::NewOnFreeList<DescriptorPool>(s_Renderer->GetDeviceWrapper(), metaData[0].UniformBlocks.size(), 1,
				VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
			m_updatedDesc = false;
			m_descPool->FreeDescriptorSets();
			m_descSet = m_descPool->AllocDescriptorSet(m_shader->GetDescLayout());
			UpdateLoadUniforms();
			UpdateUniforms();
		}

		const Shader* VulkanMaterial::GetShader()
		{
			return m_shader;
		}

		void VulkanMaterial::UpdateUniforms()
		{
			std::vector<UniformData*> tempData;
			for (auto it = m_uniformData.begin(); it != m_uniformData.end(); ++it)
			{
				tempData.push_back(&(*it).second);
			}
			m_descSet->UpdateDescriptorSet(s_Renderer->GetDevice(), tempData);
		}

		MVPUniformBuffer m_mvp;
		static int temp = 0;
		void VulkanMaterial::UpdateLoadUniforms()
		{
			UniformData data;

			if (m_uniformData.find("MVP") == m_uniformData.end())
			{
				data.Size = sizeof(m_mvp);
				data.Binding = 0;
				data.Index = 0;
				MapNewBufferMem(data);
				m_uniformData["MVP"] = data;
			}
			data = m_uniformData["MVP"];

			m_mvp.u_model = glm::rotate(m_mvp.u_model, Insight::Time::GetDeltaTime() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			memcpy(data.Data, &m_mvp, sizeof(m_mvp));

			if (m_updatedDesc == true)
			{
				m_mvp.u_view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				m_mvp.u_proj = glm::perspective(glm::radians(45.0f),
					(float)s_Renderer->m_windowModule->GetWindow()->GetWidth() / (float)s_Renderer->m_windowModule->GetWindow()->GetHeight(), 0.1f, 10.0f);
				m_mvp.u_proj[1][1] *= -1;
				return;
			}
			m_updatedDesc = true;
			m_mvp.u_model = glm::rotate(glm::mat4(1.0f), Insight::Time::GetDeltaTime() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			m_mvp.u_view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			m_mvp.u_proj = glm::perspective(glm::radians(45.0f),
				(float)s_Renderer->m_windowModule->GetWindow()->GetWidth() / (float)s_Renderer->m_windowModule->GetWindow()->GetHeight(), 0.1f, 10.0f);
			m_mvp.u_proj[1][1] *= -1;
		}

		void VulkanMaterial::UpdateLoadUniforms(const std::string& key, void* uniformData, size_t size, int binding, int index)
		{
			UniformData data;

			if (m_uniformData.find(key) == m_uniformData.end())
			{
				data.Size = size;
				data.Binding = binding;
				data.Index = index;
				MapNewBufferMem(data);
				m_uniformData[key] = data;
			}
			data = m_uniformData[key];

			memcpy(data.Data, uniformData, data.Size);
		}

		void VulkanMaterial::Bind(CommandBuffer* commandBuffers)
		{
			m_shader->Bind(commandBuffers);

			vkCmdBindDescriptorSets(commandBuffers->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_shader->GetPipelineLayout(), 0, 1, m_descSet->GetSet(), 0, nullptr);
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

		void VulkanMaterial::MapNewBufferMem(UniformData& uniformData)
		{
			CreateBufferMat(s_Renderer->GetDeviceWrapper(), uniformData.Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformData.Buffer, uniformData.BufferMem);

			vkMapMemory(s_Renderer->GetDevice(), uniformData.BufferMem, 0, uniformData.Size, 0, &uniformData.Data);
		}
	}
}