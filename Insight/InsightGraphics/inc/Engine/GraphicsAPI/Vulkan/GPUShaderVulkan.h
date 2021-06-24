#pragma once

#include "VulkanHeaders.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	struct DescriptorSetLayoutData
	{
		u32 SetNumber;
		VkDescriptorSetLayoutCreateInfo CreateInfo;
		std::vector<VkDescriptorBindingFlags> BindingFlags;
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
		VkDescriptorSetLayout Layout;
	};

	struct PipelineVertexInputState
	{
		std::vector<VkVertexInputBindingDescription> VertexInputBinding;
		std::vector<VkVertexInputAttributeDescription> VertexInputAttribute;
		VkPipelineVertexInputStateCreateInfo CreateInfo;
	};

	struct PushConstant
	{
		ShaderStage Stage;
		u32 Offset = 0;
		u32 Size = 0;
	};

	class GPUShaderVulkan : public GPUResouceVulkan<Insight::Graphics::GPUShader>
	{
	public:
		GPUShaderVulkan();
		virtual ~GPUShaderVulkan() override;

		const VkPipelineVertexInputStateCreateInfo& GetPipelineVertexInputState() const { return m_pipelineVertexInputState.CreateInfo; }
		std::vector<VkPipelineShaderStageCreateInfo> GetPipelineShaderStages();

		// [GPUShader]
		virtual void Compile() override;
		virtual void ReleaseGPUResoucesEarly() override;

		const std::unordered_map<u32, DescriptorSetLayoutData>& GetDescriptorSetlayouts() const { return m_setLayouts; }
		const PushConstant& GetPushConstant() const { return m_pushConstant; }

	protected:
		// [GPUResouce]
		virtual void OnReleaseGPU() override;
		virtual void SetName(const std::string& name) override;

	private:
		void CompileModules();
		void ParseDescriptorSetLayouts();

	private:
		std::array<VkPipelineShaderStageCreateInfo, (size_t)ShaderStage::Count> m_pipelineShaderStages;
		PipelineVertexInputState m_pipelineVertexInputState;
		std::unordered_map<u32, DescriptorSetLayoutData> m_setLayouts;
		PushConstant m_pushConstant;
	};

	class GPUPipelineVulkan : public GPUResouceVulkan<Graphics::GPUPipeline>
	{
	public:
		GPUPipelineVulkan() { }
		virtual ~GPUPipelineVulkan() override;

		virtual void SetShader(Graphics::GPUShader* shader) override;
		virtual void Init(Graphics::GPURenderGraphPass* graphPass, Graphics::GPUPipelineDesc& desc) override;

		VkPipeline GetPipeline() const { return m_pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_layout; }

		// [GPUResource]
		virtual void SetName(const std::string& name) override;
	protected:
		virtual void OnReleaseGPU() override;

	private:
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
	};
}