#pragma once

#include "../VulkanPlatformBase.h"

class Win32VulkanPlatform : public VulkanPlatformBase
{
public:
	static void GetInstanceExtensions(std::vector<const char*>& extensions);
};

using VulkanPlatform = Win32VulkanPlatform;