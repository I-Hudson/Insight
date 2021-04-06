#pragma once

#include "VulkanHeaders.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

class GPUShaderVulkan : public GPUResouceVulkan<GPUShader>
{
public:
	GPUShaderVulkan();
	virtual ~GPUShaderVulkan() override;

	const VkPipelineVertexInputStateCreateInfo& GetPipelineVertexInputState() const { return m_pipelineVertexInputState; }

	// [GPUShader]
	virtual void Compile() override;
	virtual void ReleaseGPUResoucesEarly() override;

protected:
	//[GPUResouce]
	virtual void OnReleaseGPU() override;

private:
	void CompileModules();

private:
	std::array<VkPipelineShaderStageCreateInfo, (size_t)ShaderStage::Count> m_pipelineShaderStages;
	VkPipelineVertexInputStateCreateInfo m_pipelineVertexInputState;
	
};

