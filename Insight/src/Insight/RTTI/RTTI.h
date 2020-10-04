#pragma once
#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/RTTI/RTTITypes.h"

#include <map>

namespace Insight
{
	namespace RTTI
	{
		class RTTI : public TSingleton<RTTI>
		{
		public:
			RTTI();
			~RTTI();

			template<typename T>
			T* GetProperty(void* ownerPtr, const std::string& propertyName);
			void RTTI::RegisterProperty(void* ownerObject, void* objectPtr, const std::string& propertyName, const std::string& typeName);

		private:
			IRTTIType* GetNewRTTIType(void* objectPtr, const std::string& propertyName, const std::string& typeName);

			static std::map<void*, std::vector<IRTTIType*>> m_RTTITypes;
		};

		template<typename T>
		inline T* RTTI::GetProperty(void* ownerPtr, const std::string& propertyName)
		{
			auto properties = m_RTTITypes[ownerPtr];
			if (!properties.empty())
			{
				auto typeIT = std::find_if(properties.begin(), properties.end(), [&propertyName](IRTTIType* rttiType)
					{
						if (rttiType->GetPropertyName() == propertyName)
						{
							return rttiType;
						}
					});

				IRTTIType* type = *typeIT;

				if (type)
				{
					return (T*)type;
				}
			}

			return nullptr;
		}
	}
}

#define IS_PROPERTY(type, ptr, propertyName, editorFlags) \
	Insight::RTTI::RTTI::GetInstance()->RegisterProperty(this, (void*)(&ptr), propertyName, typeid(type).name());

#define IS_GET_PROPERTY(type, propertyName) \
	Insight::RTTI::RTTI::GetInstance()->GetProperty<type>(this, propertyName);