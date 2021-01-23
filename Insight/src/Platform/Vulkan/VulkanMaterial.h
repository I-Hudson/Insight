#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Graphics/Material.h"
#include "VulkanHeader.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Engine/ShaderParser/ShaderParser.h"

#include <bitset>

	namespace Render
	{
		class Texture;
	}

struct MaterialUniformBuffer
{
	vks::VulkanBuffer Buffer;
	VkDescriptorSet Set;
	uint32_t Binding;
	VkDescriptorType Type;
	MaterialUniformDynamicBlock DynamicUniformBlock;

	MaterialUniformBuffer()
		: Binding(-1)
	{ }
};

struct MaterialTexture
{
	VkDescriptorImageInfo ImageInfo;
	VkDescriptorSet Set;
	uint32_t Binding;
	VkDescriptorType Type;
};

namespace vks
{
	struct RenderPassInfo;

	class IS_API VulkanMaterial : public Material
	{
	public:
		VulkanMaterial();
		virtual ~VulkanMaterial() override;

		virtual void CreateDefault() override;
		void Create(VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, const RenderPassInfo& renderPassInfo);
		void Destroy();

		virtual void Update() override;
		void Bind(VkCommandBuffer commandBuffer, MaterialBlockData* materialBlockData = nullptr, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

		virtual void ResetUniformInfo() override;

		virtual void UploadUniform(const std::string& key, void* data, const U32& dataSize, MaterialBlockData& materialBlockData) override;
		virtual void UploadTexture(const std::string& key, WeakPtr<Render::Texture> texture) override;
		virtual void UploadTexture(const std::string& key, void* imageView, void* sampler, const U32& format) override;

	private:
		void SetupUniformBuffers();
		void CleanUpUniformBuffers();
		void UpdateDescriptorSets();

		void CalcDynamicUniformAlig(U64& v, const U64& uniformSize);
		void CreateDynamicUniformBuffer(MaterialUniformBuffer& materialBlock, const U64& newSize);

		void FindValidMaterialBlock(MaterialUniformBuffer& materialUniformBuffer, const std::string& uniformKey, MaterialBlockData& materialBlockData);

		std::vector<VkDescriptorSet> GetDescriptorSets();

	private:
		VulkanDevice* m_device;
		std::unordered_map<std::string, MaterialUniformBuffer> m_uniformBuffers;
		std::unordered_map<std::string, MaterialTexture> m_uniformTextures;

		bool updated = false;
		bool m_init;
		std::vector<ParsedShadeData> m_shaderData;
		
		U8 m_numberOfDescriptorSets;

		VulkanPipeline m_pipeline;
	};
}