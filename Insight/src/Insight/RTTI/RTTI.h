#pragma once
#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/RTTI/RTTITypes.h"

namespace Insight
{
	class Object;

	namespace RTTI
	{
		class RTTI : public TSingleton<RTTI>
		{
		public:
			RTTI();
			~RTTI();

			void RTTI::RegisterProperty(Object* ownerObject, void* propertyPtr, const std::string& propertyName, const std::string& typeName, const RTTIPropertyEditorFlags& editorFlags);
			void RTTI::UnregisterProperty(void* ownerObject, void* propertyPtr);
			void RTTI::UnregisterAllProperty(void* ownerObject);

			RTTIProperty* GetProperty(void* ownerPtr, const std::string& propertyName);
			std::vector<RTTIProperty*> GetAllProperties(void* ownerPtr, const RTTIPropertyEditorFlags& editorFlags);

		private:
			std::unordered_map<void*, std::vector<RTTIProperty*>> m_RTTITypes;
		};
	}
}

#define IS_REGISTER_PROPERTY(type, ptr, propertyName, editorFlags) \
	Insight::RTTI::RTTI::Instance()->RegisterProperty(this, (void*)(&ptr), propertyName, typeid(type).name(), editorFlags);

#define IS_UNREGISTER_PROPERTY(ptr) \
	Insight::RTTI::RTTI::Instance()->UnregisterProperty(this, (void*)(&ptr));

#define IS_GET_PROPERTY(propertyName) \
	Insight::RTTI::RTTI::Instance()->GetProperty(this, propertyName);

#define IS_GET_ALL_PROPERTIES(ownerPtr, edtiroFlags) \
	Insight::RTTI::RTTI::Instance()->GetAllProperties(ownerPtr, edtiroFlags);