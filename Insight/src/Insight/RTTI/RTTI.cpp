#include "ispch.h"

#include "Insight/RTTI/RTTI.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace RTTI
	{
		std::map<void*, std::vector<IRTTIType*>> RTTI::m_RTTITypes;

		RTTI::RTTI()
		{
		}

		RTTI::~RTTI()
		{
			m_RTTITypes.clear();
		}

		void RTTI::RegisterProperty(void* ownerObject, void* objectPtr, const std::string& propertyName, const std::string& typeName)
		{
			IRTTIType* rttiType = GetNewRTTIType(objectPtr, propertyName, typeName);
			if (rttiType == nullptr)
			{
				IS_CORE_ERROR("[RTTI::RegisterProperty] Can't not find RTTIType.");
				return;
			}

			m_RTTITypes[ownerObject].push_back(rttiType);

			float f = *(float*)rttiType->GetObjectPtr();
			int i = ((RTTIIntType*)rttiType)->GetValue();
		}

		IRTTIType* RTTI::GetNewRTTIType(void* objectPtr, const std::string& propertyName, const std::string& typeName)
		{
#define TYPE_NAME(x) typeid(x).name()

			std::string s = TYPE_NAME(float);

			if (typeName == TYPE_NAME(std::string)) { return NEW_ON_HEAP(RTTIStringType, typeName, propertyName, objectPtr); }
			else if (typeName == TYPE_NAME(bool)) { return NEW_ON_HEAP(RTTIBoolType, typeName, propertyName, objectPtr); }
			else if (typeName == TYPE_NAME(int)) { return NEW_ON_HEAP(RTTIIntType, typeName, propertyName, objectPtr); }
			else if (typeName == TYPE_NAME(float)) { return NEW_ON_HEAP(RTTIFloatType, typeName, propertyName, objectPtr); }

			return nullptr;
		}
	}
}