#pragma once

#include "Insight/Core.h"
#include "VulkanHeader.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Insight/ShaderParser/ShaderParser.h"

namespace vks
{
	struct MaterialBlockData
	{

	};

	struct MaterialUniformBuffer
	{
		VulkanBuffer Buffer;
		VkDescriptorSet Set;
		uint32_t Binding;
		VkDescriptorType Type;
	};

	class IS_API VulkanMaterial
	{
	public:
		VulkanMaterial();
		~VulkanMaterial();

		void Create(VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass);
		void Destroy();

		void Update();
		void Bind(VkCommandBuffer commandBuffer, MaterialBlockData& materialBlockData = MaterialBlockData(), VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

		template<typename T>
		void UploadUniform(const std::string& key, T data);

	private:
		void SetupUniformBuffers();
		void CleanUpUniformBuffers();
		void UpdateDescriptorSets();

		std::vector<VkDescriptorSet> GetDescriptorSets();

	private:
		VulkanDevice* m_device;

		bool m_init;
		std::vector<Insight::ParsedShadeData> m_shaderData;
		
		std::unordered_map<std::string, MaterialUniformBuffer> m_uniformBuffers;
		VulkanPipeline m_pipeline;
	};

	template<typename T>
	inline void VulkanMaterial::UploadUniform(const std::string& key, T data)
	{
		if (m_uniformBuffers.find(key) == m_uniformBuffers.end())
		{
			return;
		}

		VulkanBuffer& ubo = m_uniformBuffers[key].Buffer;
		ubo.CopyTo(&data, sizeof(T));
	}
}