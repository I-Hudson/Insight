#include "ispch.h"
#include "QueueFamily.h"

#include "Device.h"
#include "Insight/Renderer/Vulkan.h"

namespace Insight
{
	namespace Render
	{
		QueueFamily::QueueFamily(const QueueFamilyType type, const VkPhysicalDevice device, const VkSurfaceKHR surface)
			: m_type(type)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies)
			{
				if (type == QueueFamilyType::Graphics && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					m_queueFamily = i;
				}
				else if (type == QueueFamilyType::Present)
				{
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

					if (presentSupport)
					{
						m_queueFamily = i;
					}
				}

				if (IsComplete())
				{
					break;
				}

				i++;
			}
		}

		QueueFamily::~QueueFamily()
		{ }
	}
}