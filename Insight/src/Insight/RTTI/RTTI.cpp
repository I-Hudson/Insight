#include "ispch.h"

#include "Insight/RTTI/RTTI.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Log.h"
#include "Insight/Object.h"

namespace Insight
{
	namespace RTTI
	{
		RTTI::RTTI()
		{
		}

		RTTI::~RTTI()
		{
			for (auto it = m_RTTITypes.begin(); it != m_RTTITypes.end(); ++it)
			{
				for (auto vIT = it->second.begin(); vIT != it->second.end(); ++vIT)
				{
					DELETE_ON_HEAP(*vIT);
				}
			}

			m_RTTITypes.clear();
		}

		RTTIProperty* RTTI::GetProperty(void* ownerPtr, const std::string& propertyName)
		{
			auto properties = m_RTTITypes[ownerPtr];
			if (!properties.empty())
			{
				auto typeIT = std::find_if(properties.begin(), properties.end(), [&propertyName](RTTIProperty* rttiType)
					{
						if (rttiType->GetPropertyName() == propertyName)
						{
							return rttiType;
						}
					});

				RTTIProperty* type = *typeIT;

				if (type)
				{
					return type;
				}
			}

			return nullptr;
		}

		std::vector<RTTIProperty*> RTTI::GetAllProperties(void* ownerPtr, const RTTIPropertyEditorFlags& editorFlags)
		{
			std::vector<RTTIProperty*> properties;
			for (auto it = m_RTTITypes[ownerPtr].begin(); it != m_RTTITypes[ownerPtr].end(); ++it)
			{
				if (((*it)->GetPropertyEditorFlags() & editorFlags) != 0)
				{
					properties.push_back(*it);
				}
			}
			return properties;
		}

		void RTTI::RegisterProperty(Object* ownerObject, void* propertyPtr, const std::string& propertyName, const std::string& typeName, const RTTIPropertyEditorFlags& editorFlags)
		{
			RTTIProperty* rttiType = NEW_ON_HEAP(RTTIProperty, typeName, propertyName, editorFlags, propertyPtr);
			if (rttiType == nullptr)
			{
				IS_CORE_ERROR("[RTTI::RegisterProperty] Can't not find RTTIType.");
				return;
			}

			m_RTTITypes[ownerObject].push_back(rttiType);

			// Register remove from RTTI when object is destroyed.
			ownerObject->RegisterOnDestroyCallback(this, [&](Object* objectPtr)
				{
					objectPtr->UnregisterOnDestroyCallback(this);
					RTTI::Instance()->UnregisterAllProperty(objectPtr);
				});
		}

		void RTTI::UnregisterProperty(void* ownerObject, void* propertyPtr)
		{
			std::vector<RTTIProperty*>& properties = m_RTTITypes[ownerObject];
			auto typeIT = std::find_if(properties.begin(), properties.end(), [&propertyPtr](RTTIProperty* rttiType)
				{
					if (rttiType->GetObjectPtr() == propertyPtr)
					{
						return rttiType;
					}
				});

			if (typeIT != properties.end())
			{
				DELETE_ON_HEAP(*typeIT);
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
				std::vector<RTTIProperty*>& properties = m_RTTITypes[ownerObject];
				for (auto it = properties.begin(); it != properties.end(); ++it)
				{
					DELETE_ON_HEAP(*it);
				}
				properties.clear();
				m_RTTITypes.erase(ownerObject);
			}
		}
	}
}