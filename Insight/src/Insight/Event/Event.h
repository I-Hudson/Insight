#pragma once

#include "Insight/Core/Core.h"
#include <ostream>
#include <sstream>
#include <string>

enum class EventType
{
	None = 0,

	GraphicsInit,

	WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
	AppTick, AppUpdate, AppRender,
	KeyPressed, KeyReleased, KeyTyped,
	MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,

	Serialize, Deserialize,

	VulkanWindowResize,
};

static std::string EventTypeToString(const EventType& type)
{
	switch (type)
	{
	case EventType::GraphicsInit: return "GraphicsInit";
	case EventType::WindowClose: return "WindowClose";
	case EventType::WindowResize: return "WindowResize";
	case EventType::WindowFocus: return "WindowFocus";
	case EventType::WindowLostFocus: return "WindowLostFocus";
	case EventType::WindowMoved: return "WindowMoved";
	case EventType::AppTick: return "AppTick";
	case EventType::AppUpdate: return "AppUpdate";
	case EventType::AppRender: return "AppRender";
	case EventType::KeyPressed: return "KeyPressed";
	case EventType::KeyReleased: return "KeyReleased";
	case EventType::KeyTyped: return "KeyTyped";
	case EventType::MouseButtonPressed: return "MouseButtonPressed";
	case EventType::MouseButtonReleased: return "MouseButtonReleased";
	case EventType::MouseMoved: return "MouseMoved";
	case EventType::MouseScrolled: return "MouseScrolled";
	case EventType::Serialize: return "Serialize";
	case EventType::Deserialize: return "Deserialize";
	default: return "Type is unknow.";
	}
}

#define BIT(x) (1 << x)

namespace
{
	enum EventCategory
	{
		EventCategoryNone = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};
}

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

class IS_API Event
{
public:
	bool Handled = false;

	virtual EventType GetEventType() const = 0;
	virtual const char* GetName() const = 0;
	virtual int GetCategoryFlags() const = 0;
	virtual std::string ToString() const { return GetName(); }

	inline bool IsInCatgeory(EventCategory a_catgeory)
	{
		return GetCategoryFlags() & a_catgeory;
	}
};

inline std::ostream& operator<< (std::ostream& a_os, const Event& a_event)
{
	return a_os << a_event.ToString();
}
