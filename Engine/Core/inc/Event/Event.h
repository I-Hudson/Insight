#pragma once

namespace Insight
{
	namespace Core
	{
		enum class EventType
		{
			Graphics_Swapchain_Resize,
			Graphics_Render_Resolution_Change,
		};

		struct IS_CORE Event
		{
			virtual std::string GetName() = 0;
			virtual EventType GetEventType() = 0;
		};

		struct IS_CORE GraphcisSwapchainResize : public Event
		{
			GraphcisSwapchainResize() { }
			GraphcisSwapchainResize(int width, int height)
				: Width(width), Height(height)
			{ }

			virtual std::string GetName() override { return "GraphcisSwapchainResize"; }
			virtual EventType GetEventType() override { return EventType::Graphics_Swapchain_Resize; }

			int Width;
			int Height;
		};

		struct IS_CORE GraphicsRenderResolutionChange : public Event
		{
			GraphicsRenderResolutionChange() { }
			GraphicsRenderResolutionChange(int width, int height)
				: Width(width), Height(height)
			{ }

			virtual std::string GetName() override { return "GraphicsRenderResolutionChange"; }
			virtual EventType GetEventType() override { return EventType::Graphics_Render_Resolution_Change; }

			int Width;
			int Height;
		};
	}
}