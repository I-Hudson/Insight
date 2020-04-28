#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"

#include <optional>

namespace Insight
{
	namespace Render
	{
		class Device;
		enum class QueueFamilyType
		{
			None,
			Graphics,
			Present
		};

		class IS_API QueueFamily
		{
		public:
			QueueFamily() { }
			QueueFamily(const QueueFamilyType type, const VkPhysicalDevice device, const VkSurfaceKHR surface);
			~QueueFamily();

			const QueueFamilyType GetType() const { return m_type; }
			const bool IsComplete() const { return m_queueFamily.has_value(); }
			const uint32_t GetValue() const { return m_queueFamily.value(); }

		private:
			QueueFamilyType m_type;
			std::optional<uint32_t> m_queueFamily;
		};
	}
}