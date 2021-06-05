#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/Win32/Win32VulkanPlatform.h"
#include "vulkan/vulkan_win32.h"

void Win32VulkanPlatform::GetInstanceExtensions(std::vector<const char*>& extensions)
{
    // Include Windows surface extension
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}
