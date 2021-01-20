#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Event/Event.h"
#include <string>

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

			std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Close";
			return ss.str();
		}
	};

	class WindowResizeEvent : public Event
	{
	public:

		WindowResizeEvent(unsigned int a_width, unsigned int a_height)
			: m_width(a_width), m_height(a_height)
		{}

		EVENT_CLASS_TYPE(WindowResize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

			std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Resize Event: " << m_width << ", " << m_height << "\n";
			return ss.str();
		}
	//private:
		unsigned int m_width, m_height;
	};

	class SerializeEvent : public Event
	{
	public:

		SerializeEvent()
		{ }

		EVENT_CLASS_TYPE(Serialize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

			std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Serialize" << '\n';
			return ss.str();
		}
	};

	class DeserializeEvent : public Event
	{
	public:

		DeserializeEvent()
		{ }

		EVENT_CLASS_TYPE(Deserialize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

			std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Deserialize" << '\n';
			return ss.str();
		}
	};

	class VulkanResizeEvent : public Event
	{
	public:

		VulkanResizeEvent(unsigned int a_width, unsigned int a_height)
			: m_width(a_width), m_height(a_height)
		{
		}

		EVENT_CLASS_TYPE(VulkanWindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

			std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Resize Event: " << m_width << ", " << m_height << "\n";
			return ss.str();
		}
		//private:
		unsigned int m_width, m_height;
	};