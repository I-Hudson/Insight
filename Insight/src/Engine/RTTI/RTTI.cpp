#include "ispch.h"

#include "Engine/RTTI/RTTI.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Object.h"

namespace RTTI
{
	RTTI::RTTI()
	{
	}

	RTTI::~RTTI()
	{
		m_RTTITypes.clear();
	}

	const RTTIProperty& RTTI::GetProperty(void* ownerPtr, const std::string& propertyName)
	{
		auto& properties = m_RTTITypes[ownerPtr];
		if (!properties.empty())
		{
			auto typeIT = std::find_if(properties.begin(), properties.end(), [&propertyName](RTTIProperty& rttiType)
			{
				return rttiType.GetPropertyName() == propertyName;
			});

			if (typeIT != properties.end())
			{
				return *typeIT;
			}
		}
		return RTTIProperty("Invalid", "Invalid", 0, nullptr);
	}

	const std::vector<RTTIProperty*> RTTI::GetAllProperties(void* ownerPtr, const uint32_t& editorFlags)
	{
		std::vector<RTTIProperty*> properties;
		for (auto it = m_RTTITypes[ownerPtr].begin(); it != m_RTTITypes[ownerPtr].end(); ++it)
		{
			if (((*it).GetPropertyFlags() & editorFlags) != 0)
			{
				properties.push_back(&(*it));
			}
		}
		return properties;
	}

	void RTTI::RegisterProperty(Object* ownerObject, void* propertyPtr, const std::string& propertyName, const std::string& typeName, const uint32_t& editorFlags)
	{
		RTTIProperty rttiType = RTTIProperty(typeName, propertyName, editorFlags, propertyPtr);
		if (!rttiType.IsValid())
		{
			IS_CORE_ERROR("[RTTI::RegisterProperty] Can't not find RTTIType.");
			return;
		}

		m_RTTITypes[ownerObject].push_back(rttiType);

		// Register remove from RTTI when object is destroyed.
		ownerObject->RegisterOnDestroyCallback(this, [&](Object* objectPtr)
		{
			//objectPtr->UnregisterOnDestroyCallback(this);
			RTTI::Instance()->UnregisterAllProperty(objectPtr);
		});
	}

	void RTTI::UnregisterProperty(void* ownerObject, void* propertyPtr)
	{
		std::vector<RTTIProperty>& properties = m_RTTITypes[ownerObject];
		auto typeIT = std::find_if(properties.begin(), properties.end(), [&propertyPtr](RTTIProperty& rttiType)
		{
			return rttiType.GetObjectPtr() == propertyPtr;
		});

		if (typeIT != properties.end())
		{
			properties.erase(typeIT);

			if (properties.size() == 0)
			{
				m_RTTITypes.erase(ownerObject);
				return;
			}
		}
	}

	void RTTI::UnregisterAllProperty(void* ownerObject)
	{
		if (m_RTTITypes.find(ownerObject) != m_RTTITypes.end())
		{
			std::vector<RTTIProperty>& properties = m_RTTITypes[ownerObject];
			properties.clear();
			m_RTTITypes.erase(ownerObject);
		}
	}
}