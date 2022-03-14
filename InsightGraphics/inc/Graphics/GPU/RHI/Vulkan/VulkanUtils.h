#pragma once

#include "Graphics/PixelFormat.h"
#include <vulkan/vulkan.hpp>

extern vk::Format PixelFormatToVkFormat[static_cast<int>(PixelFormat::MAX)];
extern PixelFormat VkFormatToPixelFormat[static_cast<int>(PixelFormat::MAX)];
