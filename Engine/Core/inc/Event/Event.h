#pragma once

#include "Core/Defines.h"

namespace Insight
{
	namespace Core
	{
		enum class EventType
		{
			Graphics_Swapchain_Resize,
			Graphics_Render_Resolution_Change,

			Project_Open,
			Project_Save,

			WorldSave,
			WorldLoad,
			WorldDestroy,

			Size
		};

		bool EventTypeMultiplePerFrame[] =
		{
			false,	// Graphics_Swapchain_Resize
			false,  // Graphics_Render_Resolution_Change

			true,	// Project_Open
			true,	// Project_Save
			
			true,	// WorldSave
			true,	// WorldLoad
			true,	// WorldDestroy
		};
		static_assert(ARRAY_COUNT(EventTypeMultiplePerFrame) == static_cast<u64>(EventType::Size));

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

		struct IS_CORE ProjectOpenEvent : public Event
		{
			ProjectOpenEvent() { }
			ProjectOpenEvent(std::string projectPath)
				: ProjectPath(std::move(ProjectPath))
			{ }

			virtual std::string GetName() override { return "ProjectOpenEvent"; }
			virtual EventType GetEventType() override { return EventType::Project_Open; }

			std::string ProjectPath;
		};
	}
}