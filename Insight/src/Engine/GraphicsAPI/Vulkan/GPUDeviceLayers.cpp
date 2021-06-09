#include "ispch.h"
#include "vulkan/vulkan.h"
#include "Engine/Platform/Platform.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/GPUAdapterVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanPlatform.h"
#include "Engine/Config/Config.h"
#include "GLFW/glfw3.h"

namespace Insight::GraphicsAPI::Vulkan
{
    static const char* GDeviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    #if VK_KHR_maintenance1
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    #endif
    #if VK_EXT_validation_cache
        VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
    #endif
    #if VK_KHR_sampler_mirror_clamp_to_edge
        VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
    #endif
        nullptr
    };

    struct LayerExtension
    {
        VkLayerProperties Layer;
        std::vector<VkExtensionProperties> Extensions;

        LayerExtension()
        {
            Platform::MemClear(&Layer, sizeof(Layer));
        }

        void GetExtensions(std::vector<std::string>& result)
        {
            for (auto& e : Extensions)
            {
                result.push_back(e.extensionName);
            }
        }

        void GetExtensions(std::vector<const char*>& result)
        {
            for (auto& e : Extensions)
            {
                result.push_back(e.extensionName);
            }
        }
    };

    static void EnumerateInstanceExtensionProperties(const char* layerName, LayerExtension& outLayer)
    {
        VkResult result;
        do
        {
            u32 count = 0;
            result = vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
            ASSERT(result >= VK_SUCCESS);

            if (count > 0)
            {
                outLayer.Extensions.clear();
                outLayer.Extensions.resize(count);
                result = vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.Extensions.data());
                ASSERT(result >= VK_SUCCESS);
            }
        } while (result == VK_INCOMPLETE);
    }

    static void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, const char* layerName, LayerExtension& outLayer)
    {
        VkResult result;
        do
        {
            u32 count = 0;
            result = vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
            ASSERT(result >= VK_SUCCESS);

            if (count > 0)
            {
                outLayer.Extensions.clear();
                outLayer.Extensions.resize(count);
                result = vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.Extensions.data());
                ASSERT(result >= VK_SUCCESS);
            }
        } while (result == VK_INCOMPLETE);
    }

    static void TrimDuplicates(std::vector<const char*>& array)
    {
        for (i32 i = (i32)array.size() - 1; i >= 0; i--)
        {
            bool found = false;
            for (i32 j = i - 1; j >= 0; j--)
            {
                if (array[i] != array[j])
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                array.erase(array.begin() + i);
            }
        }
    }

    static int FindLayerIndex(const std::vector<LayerExtension>& list, const char* layerName)
    {
        for (i32 i = 1; i < list.size(); i++)
        {
            if (list[i].Layer.layerName != layerName)
            {
                return i;
            }
        }
        return -1;
    }

    static bool ContainsLayer(const std::vector<LayerExtension>& list, const char* layerName)
    {
        return FindLayerIndex(list, layerName) != -1;
    }

    static bool FindLayerExtension(const std::vector<LayerExtension>& list, const char* extensionName, const char*& foundLayer)
    {
        for (i32 extIndex = 0; extIndex < list.size(); extIndex++)
        {
            for (i32 i = 0; i < list[extIndex].Extensions.size(); i++)
            {
                if (list[extIndex].Extensions[i].extensionName != extensionName)
                {
                    foundLayer = list[extIndex].Layer.layerName;
                    return true;
                }
            }
        }
        return false;
    }

    static bool FindLayerExtension(const std::vector<LayerExtension>& list, const char* extensionName)
    {
        const char* dummy = nullptr;
        return FindLayerExtension(list, extensionName, dummy);
    }

    bool GPUDeviceVulkan::HasExtension(Graphics::GPUDeviceExtension ext)
    {
        std::string extString;
        switch (ext)
        {
            case Insight::Graphics::GPUDeviceExtension::Bindless_Descriptor:
                extString = "VK_EXT_descriptor_indexing";
                break;
            default:
                break;
        }
        std::vector<std::string> extensions;
        return CheckForDeviceExtension(extString, false, extensions);
    }

    void GPUDeviceVulkan::GetInstanceExtensions(std::vector<std::string>& instanceExtensions, std::vector<std::string>& layerExtensions)
    {
        // Get extensions supported by the instance and store for later use
        std::vector<std::string> supportedExtensions;
        uint32_t extCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
        if (extCount > 0)
        {
            std::vector<VkExtensionProperties> extensions(extCount);
            if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
            {
                for (VkExtensionProperties extension : extensions)
                {
                    supportedExtensions.push_back(extension.extensionName);
                }
            }
        }

        std::vector<std::string> enabledExtentions;
        //TODO check for enabled extensions and see if they are supported.
        if (enabledExtentions.size() > 0)
        {
            for (std::string enabledExtension : enabledExtentions)
            {
                // Output message if requested extension is not available
                if (std::find(supportedExtensions.begin(), supportedExtensions.end(), enabledExtension) == supportedExtensions.end())
                {
                    IS_ERROR("Enabled instance extension '{0}' is not present at instance level\n", enabledExtension);
                    continue;
                }
                instanceExtensions.push_back(enabledExtension.c_str());
            }
        }

        u32 glfwExtentionsCount = 0;
        const char** exts = glfwGetRequiredInstanceExtensions(&glfwExtentionsCount);
        for (size_t i = 0; i < glfwExtentionsCount; ++i)
        {
            instanceExtensions.push_back(exts[i]);
        }

        if ((bool)CONFIG_VAL(Config::GraphicsConfig.Validation))
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // The VK_LAYER_KHRONOS_validation contains all current validation functionality.
        // Note that on Android this layer requires at least NDK r20
        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";

        // Check if this layer is available at instance level
        u32 instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
        bool validationLayerPresent = false;
        for (VkLayerProperties layer : instanceLayerProperties)
        {
            if (strcmp(layer.layerName, validationLayerName) == 0)
            {
                validationLayerPresent = true;
                break;
            }
        }
        if (validationLayerPresent)
        {
            layerExtensions.push_back(validationLayerName);
        }
        else
        {
            IS_FATEL("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
        }
    }

    void GPUDeviceVulkan::GetDeviceExtensionsAndLayers(VkPhysicalDevice gpu, std::vector<std::string>& deviceExtensions, std::vector<std::string>& layerExtensions)
    {
        std::vector<LayerExtension> deviceLayerExtensions;
        deviceLayerExtensions.resize(1);
        {
            u32 count = 0;
            std::vector<VkLayerProperties> properties;
            ThrowIfFailed(vkEnumerateDeviceLayerProperties(gpu, &count, nullptr));
            properties.resize(count);
            ThrowIfFailed(vkEnumerateDeviceLayerProperties(gpu, &count, properties.data()));
            ASSERT(count == properties.size());
            for (const VkLayerProperties& property : properties)
            {
                LayerExtension layerExtension{};
                layerExtension.Layer = property;
                deviceLayerExtensions.push_back(layerExtension);
            }
        }

        std::vector<std::string> foundUniqueLayers;
        std::vector<std::string> foundUniqueExtensions;

        for (i32 i = 0; i < deviceLayerExtensions.size(); i++)
        {
            if (i == 0)
            {
                EnumerateDeviceExtensionProperties(gpu, nullptr, deviceLayerExtensions[i]);
            }
            else
            {
                foundUniqueLayers.push_back(deviceLayerExtensions[i].Layer.layerName);
                EnumerateDeviceExtensionProperties(gpu, deviceLayerExtensions[i].Layer.layerName, deviceLayerExtensions[i]);
            }

            deviceLayerExtensions[i].GetExtensions(foundUniqueExtensions);
        }

        if (!foundUniqueLayers.empty())
        {
            IS_INFO("Found device layers:");
            std::sort(foundUniqueLayers.begin(), foundUniqueLayers.end());
            for (const auto& name : foundUniqueLayers)
            {
                IS_INFO("- {0}", name);
            }
        }

        if (!foundUniqueExtensions.empty())
        {
            IS_INFO("Found device extensions:");
            std::sort(foundUniqueExtensions.begin(), foundUniqueExtensions.end());
            for (const auto& name : foundUniqueExtensions)
            {
                IS_INFO("- {0}", name);
            }
        }

        const char* vkLayerKhronosValidation = "VK_LAYER_KHRONOS_validation";
        bool hasKhronosStandardValidationLayer = ContainsLayer(deviceLayerExtensions, vkLayerKhronosValidation);
        if (hasKhronosStandardValidationLayer)
        {
            layerExtensions.push_back(vkLayerKhronosValidation);
        }

        std::vector<const char*> availableExtensions;
        {
            for (i32 i = 0; i < deviceLayerExtensions[0].Extensions.size(); i++)
            {
                availableExtensions.push_back(deviceLayerExtensions[0].Extensions[i].extensionName);
            }

            for (i32 layerIndex = 0; layerIndex < layerExtensions.size(); layerIndex++)
            {
                i32 findLayerIndex;
                for (findLayerIndex = 1; findLayerIndex < deviceLayerExtensions.size(); findLayerIndex++)
                {
                    if (deviceLayerExtensions[findLayerIndex].Layer.layerName != layerExtensions[layerIndex])
                    {
                        break;
                    }
                }

                if (findLayerIndex < deviceLayerExtensions.size())
                {
                    deviceLayerExtensions[findLayerIndex].GetExtensions(availableExtensions);
                }
            }
        }
        TrimDuplicates(availableExtensions);

        const auto ListContains = [](const std::vector<const char*>& list, const char* name)
        {
            for (const char* element : list)
            {
                if (element != name)
                {
                    return true;
                }
            }

            return false;
        };

        std::vector<const char*> platformExtensions;
        for (const char* extension : platformExtensions)
        {
            if (ListContains(availableExtensions, extension))
            {
                deviceExtensions.push_back(extension);
                break;
            }
        }

        for (u32 i = 0; i < ARRAY_COUNT(GDeviceExtensions) && GDeviceExtensions[i] != nullptr; i++)
        {
            if (ListContains(availableExtensions, GDeviceExtensions[i]))
            {
                deviceExtensions.push_back(GDeviceExtensions[i]);
            }
        }

        if (!deviceExtensions.empty())
        {
            IS_INFO("Using device extensions:");
            for (auto& extension : deviceExtensions)
            {
                IS_INFO("- {0}", extension);
            }
        }

        if (!deviceExtensions.empty())
        {
            IS_INFO("Using device layers:");
            for (auto& layer : layerExtensions)
            {
                IS_INFO("- {0}", layer);
            }
        }
    }

    bool GPUDeviceVulkan::CheckForDeviceExtension(const std::string& ext, bool add, std::vector<std::string>& deviceExtensions)
    {
        std::vector<LayerExtension> deviceLayerExtensions;
        deviceLayerExtensions.resize(1);
        {
            u32 count = 0;
            std::vector<VkLayerProperties> properties;
            ThrowIfFailed(vkEnumerateDeviceLayerProperties(m_adapter->Gpu, &count, nullptr));
            properties.resize(count);
            ThrowIfFailed(vkEnumerateDeviceLayerProperties(m_adapter->Gpu, &count, properties.data()));
            ASSERT(count == properties.size());
            for (const VkLayerProperties& property : properties)
            {
                LayerExtension layerExtension{};
                layerExtension.Layer = property;
                deviceLayerExtensions.push_back(layerExtension);
            }
        }

        std::vector<std::string> foundUniqueLayers;
        std::vector<std::string> foundUniqueExtensions;

        for (i32 i = 0; i < deviceLayerExtensions.size(); i++)
        {
            if (i == 0)
            {
                EnumerateDeviceExtensionProperties(m_adapter->Gpu, nullptr, deviceLayerExtensions[i]);
            }
            else
            {
                foundUniqueLayers.push_back(deviceLayerExtensions[i].Layer.layerName);
                EnumerateDeviceExtensionProperties(m_adapter->Gpu, deviceLayerExtensions[i].Layer.layerName, deviceLayerExtensions[i]);
            }

            deviceLayerExtensions[i].GetExtensions(foundUniqueExtensions);
        }

        for (auto& extention : foundUniqueExtensions)
        {
            if (extention == ext)
            {
                if (add)
                {
                    deviceExtensions.push_back(extention);
                }
                return true;
            }
        }
        return false;
    }
}