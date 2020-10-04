#pragma once
#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/RTTI/RTTITypes.h"

namespace Insight
{
	namespace RTTI
	{
		class RTTI : public TSingleton<RTTI>
		{
		public:
			RTTI();
			~RTTI();

			void RTTI::RegisterProperty(void* ownerObject, void* objectPtr, const std::string& propertyName, const std::string& typeName, const RTTIPropertyEditorFlags& editorFlags);
			RTTIProperty* GetProperty(void* ownerPtr, const std::string& propertyName);
			std::vector<RTTIProperty*> GetAllProperties(void* ownerPtr, const RTTIPropertyEditorFlags& editorFlags);

		private:
			std::unordered_map<void*, std::vector<RTTIProperty*>> m_RTTITypes;
		};
	}
}

#define IS_PROPERTY(type, ptr, propertyName, editorFlags) \
	Insight::RTTI::RTTI::GetInstance()->RegisterProperty(this, (void*)(&ptr), propertyName, typeid(type).name(), editorFlags);

#define IS_GET_PROPERTY(propertyName) \
	Insight::RTTI::RTTI::GetInstance()->GetProperty(this, propertyName);

#define IS_GET_ALL_PROPERTIES(ownerPtr, edtiroFlags) \
	Insight::RTTI::RTTI::GetInstance()->GetAllProperties(ownerPtr, edtiroFlags);