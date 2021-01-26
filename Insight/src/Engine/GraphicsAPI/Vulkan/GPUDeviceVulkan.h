#pragma once

#include "Engine/Graphics/GPUDevice.h"
#include "vulkan/vulkan.h"
#include "VulkanDebug.h"

class GPUAdapterVulkan;

class GPUDeviceVulkan : public GPUDevice
{
public:
    static GPUDevice* New();

	GPUDeviceVulkan();
	virtual ~GPUDeviceVulkan() override;

    virtual GPUContext* GetMainContext() override;
    virtual GPUAdapter* GetAdapter() override;
    virtual bool Init();
    virtual bool LoadContent();
    virtual bool CanDraw() { return true; }
    virtual void Dispose();
    virtual void WaitForGPU() override;

    virtual GPUBuffer* NewBuffer() override;
    virtual GPUTexture* NewTexture() override;

private:
    static void GetInstanceExtensions(std::vector<const char*>& instanceExtensions, std::vector<const char*>& layerExtensions);
    void GetDeviceExtensionsAndLayers(VkPhysicalDevice gpu, std::vector<const char*>& deviceExtensions, std::vector<const char*>& layerExtensions);


private:
    VkInstance m_instance;

    VkDevice m_device;

    std::vector<VkQueueFamilyProperties> m_queueFamilyProps;

    VkPhysicalDeviceFeatures m_physicalDeviceFeatures;

    GPUAdapterVulkan* m_adapter;
};