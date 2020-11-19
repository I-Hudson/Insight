#include "ispch.h"
#include "VulkanMaterial.h"
#include "VulkanDevice.h"

namespace vks
{
	VulkanMaterial::VulkanMaterial()
	{
	}

	VulkanMaterial::~VulkanMaterial()
	{
	}

	void VulkanMaterial::Create(VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass)
	{
		m_init = true;
		m_device = device;
		for (auto& shader : shaders)
		{
			m_shaderData.push_back(Insight::ShaderParser::ParseShader(shader));
		}
		m_pipeline.Create(device, shaders, renderPass, m_shaderData);

		SetupUniformBuffers();
	}

	void VulkanMaterial::Destroy()
	{
		m_init = false;
		m_pipeline.Destroy();
		CleanUpUniformBuffers();
	}

	void VulkanMaterial::Update()
	{
		IS_PROFILE_FUNCTION();

		if (m_init)
		{
			UpdateDescriptorSets();
		}
	}

	void VulkanMaterial::Bind(VkCommandBuffer commandBuffer, MaterialBlockData& materialBlockData, VkPipelineBindPoint bindPoint)
	{
		IS_PROFILE_FUNCTION();

		if (m_init)
		{
			std::vector<VkDescriptorSet> sets = GetDescriptorSets();
			m_pipeline.Bind(commandBuffer, bindPoint);
			vkCmdBindDescriptorSets(commandBuffer, bindPoint, m_pipeline.GetPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
		}
	}

	void VulkanMaterial::SetupUniformBuffers()
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		for (auto& shader : m_shaderData)
		{
			for (auto& uniformBlock : shader.UniformBlocks)
			{
				std::string key = uniformBlock.Name;
				if (uniformBlock.Type == Insight::ShaderUniformBlockType::UniformBuffer)
				{
					VulkanBuffer ubo;
					ThrowIfFailed(m_device->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
														 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
														 uniformBlock.Size, &ubo));
					ThrowIfFailed(ubo.Map());

					m_uniformBuffers[key].Buffer = ubo;
					m_uniformBuffers[key].Binding = uniformBlock.Binding;
					m_uniformBuffers[key].Type = uniformBlock.GetVulkanType();
				}

				// Allocates an empty descriptor set without actual descriptors from the pool using the set layout
				VkDescriptorSetAllocateInfo allocateInfo{};
				allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocateInfo.descriptorPool = m_device->GetDescriptorPool();
				allocateInfo.descriptorSetCount = 1;
				allocateInfo.pSetLayouts = &m_pipeline.GetDescriptorLayout();
				ThrowIfFailed(vkAllocateDescriptorSets(*m_device, &allocateInfo, &m_uniformBuffers[key].Set));
				writeDescriptorSets.push_back(vks::initializers::writeDescriptorSet(m_uniformBuffers[key].Set, uniformBlock.GetVulkanType(), uniformBlock.Binding, &m_uniformBuffers[key].Buffer.descriptor, 1));
			}
		}

		/*
			Execute the writes to update descriptors for this set
			Note that it's also possible to gather all writes and only run updates once, even for multiple sets
			This is possible because each VkWriteDescriptorSet also contains the destination set to be updated
		*/
		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
	void VulkanMaterial::CleanUpUniformBuffers()
	{
		for (auto& ubo : m_uniformBuffers)
		{
			ubo.second.Buffer.Unmap();
			ubo.second.Buffer.Destroy();
		}
		std::vector<VkDescriptorSet> sets = GetDescriptorSets();
		vkFreeDescriptorSets(*m_device, m_device->GetDescriptorPool(), static_cast<uint32_t>(sets.size()), sets.data());
	}

	void VulkanMaterial::UpdateDescriptorSets()
	{
		std::vector<VkWriteDescriptorSet> writesSets;

		for (auto& ubo : m_uniformBuffers)
		{
			writesSets.push_back(vks::initializers::writeDescriptorSet(ubo.second.Set, ubo.second.Type, ubo.second.Binding, &ubo.second.Buffer.descriptor));
		}

		vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(writesSets.size()), writesSets.data(), 0, nullptr);
	}

	std::vector<VkDescriptorSet> VulkanMaterial::GetDescriptorSets()
	{
		std::vector<VkDescriptorSet> sets;
		for (auto& ubo : m_uniformBuffers)
		{
			sets.push_back(ubo.second.Set);
		}
		return sets;
	}
}