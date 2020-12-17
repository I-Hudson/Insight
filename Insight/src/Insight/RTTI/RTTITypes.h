#pragma once
#include "Insight/Core.h"
#include "Insight/Enums/PropertyFlags.h"
#include <map>

namespace Insight
{
	class RTTIProperty : public Insight::Object
	{
	public:
		RTTIProperty(const std::string& typeName, const std::string propertyName, const PropertyFlags& propertyFlags, void* objectPtr)
			: m_typeName(typeName)
			, m_propertyName(propertyName)
			, m_propertyFlags(propertyFlags)
			, m_objectPtr(objectPtr)
		{
		}

		~RTTIProperty()
		{
			m_objectPtr = nullptr;
		}

		virtual std::string GetTypeName() const { return "RTTIProperty"; }

		void* GetObjectPtr()
		{
			return m_objectPtr;
		}

		template<typename T>
		T& GetPropertyValue()
		{
			return *static_cast<T*>(m_objectPtr);
		}

		const PropertyFlags GetPropertyFlags()
		{
			return m_propertyFlags;
		}

		const std::string GetPropertyName()
		{
			return m_propertyName;
		}

		const std::string GetType()
		{
			return m_typeName;
		}

		const bool IsValid()
		{
			return m_objectPtr != nullptr;
		}

	protected:
		std::string m_typeName;
		std::string m_propertyName;
		PropertyFlags m_propertyFlags;
		void* m_objectPtr;
	};
}