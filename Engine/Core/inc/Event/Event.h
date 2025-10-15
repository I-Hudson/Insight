#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

namespace Insight
{
	namespace Core
	{
		enum class EventType
		{
			Graphics_Swapchain_Resize,
			Graphics_Render_Resolution_Change,

			Project_Create,
			Project_Open,
			Project_Close,
			Project_Save,

			WorldSave,
			WorldLoad,
			WorldDestroy,

			HotReloadStarted,
			HotReloadFinished,
			HotReloadLibraryUnLoaded,
			HotReloadLibraryLoaded,

			Size
		};

		constexpr static bool EventTypeMultiplePerFrame[] =
		{
			false,	// Graphics_Swapchain_Resize
			false,  // Graphics_Render_Resolution_Change

			true,	// Project_Create
			true,	// Project_Open
			true,	// Project_Close
			true,	// Project_Save
			
			true,	// WorldSave
			true,	// WorldLoad
			true,	// WorldDestroy

			false,	// HotReloadStarted
			false,	// HotReloadFinished
			false,	// HotReloadLibraryUnLoaded
			false,	// HotReloadLibraryLoaded
		};
		static_assert(ARRAY_COUNT(EventTypeMultiplePerFrame) == static_cast<u64>(EventType::Size));

		struct IS_CORE Event
		{
			virtual ~Event() { }
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

		struct IS_CORE ProjectCreateEvent : public Event
		{
			ProjectCreateEvent() { }
			ProjectCreateEvent(const std::string projectPath)
				: ProjectPath(std::move(projectPath))
			{ }

			virtual std::string GetName() override { return "ProjectCreateEvent"; }
			virtual EventType GetEventType() override { return EventType::Project_Create; }

			std::string ProjectPath;
		};

		struct IS_CORE ProjectOpenEvent : public Event
		{
			ProjectOpenEvent() { }
			ProjectOpenEvent(const std::string projectPath)
				: ProjectPath(std::move(projectPath))
			{ }

			virtual std::string GetName() override { return "ProjectOpenEvent"; }
			virtual EventType GetEventType() override { return EventType::Project_Open; }

			std::string ProjectPath;
		};

		struct IS_CORE ProjectSaveEvent : public Event
		{
			ProjectSaveEvent() { }
			ProjectSaveEvent(const std::string projectPath)
				: ProjectPath(std::move(projectPath))
			{ }

			virtual std::string GetName() override { return "ProjectSaveEvent"; }
			virtual EventType GetEventType() override { return EventType::Project_Save; }

			std::string ProjectPath;
		};

		struct IS_CORE ProjectCloseEvent : public Event
		{
			ProjectCloseEvent() { }
			ProjectCloseEvent(const std::string projectPath)
				: ProjectPath(std::move(projectPath))
			{ }

			virtual std::string GetName() override { return "ProjectCloseEvent"; }
			virtual EventType GetEventType() override { return EventType::Project_Close; }

			std::string ProjectPath;
		};

		struct IS_CORE HotReloadStartedEvent : public Event
		{
			HotReloadStartedEvent() { }
			virtual std::string GetName() override { return "HotReloadStartedEvent"; }
			virtual EventType GetEventType() override { return EventType::HotReloadStarted; }
		};

		struct IS_CORE HotReloadFinishedEvent : public Event
		{
			HotReloadFinishedEvent() {}
			virtual std::string GetName() override { return "HotReloadFinishedEvent"; }
			virtual EventType GetEventType() override { return EventType::HotReloadFinished; }
		};

		struct IS_CORE HotReloadUnloadedEvent : public Event
		{
			HotReloadUnloadedEvent() {}
			virtual std::string GetName() override { return "HotReloadUnloadedEvent"; }
			virtual EventType GetEventType() override { return EventType::HotReloadLibraryUnLoaded; }
		};

		struct IS_CORE HotReloadLoadedEvent : public Event
		{
			HotReloadLoadedEvent() {}
			virtual std::string GetName() override { return "HotReloadLoadedEvent"; }
			virtual EventType GetEventType() override { return EventType::HotReloadLibraryLoaded; }
		};
	}
}