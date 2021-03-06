#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/RTTI/RTTITypes.h"

class Object;

namespace RTTI
{
	struct RTTIPropertyParser
	{

	};

	class RTTI : public Insight::Core::TSingleton<RTTI>
	{
	public:
		RTTI();
		~RTTI();

		void RTTI::RegisterProperty(Object* ownerObject, void* propertyPtr, const std::string& propertyName, const std::string& typeName, const u32& editorFlags = IS_PropertyFlags::None);
		void RTTI::UnregisterProperty(void* ownerObject, void* propertyPtr);
		void RTTI::UnregisterAllProperty(void* ownerObject);

		const RTTIProperty& GetProperty(void* ownerPtr, const std::string& propertyName);
		const std::vector<RTTIProperty*> GetAllProperties(void* ownerPtr, const uint32_t& editorFlags);

	private:
		std::unordered_map<void*, std::vector<RTTIProperty>> m_RTTITypes;
	};
}
//
// Helper macros for concatenating two names together such that they will defeat the macro parser.
// Useful for things like:
// #define LINEBASEDOBJ(NAME) CLineBasedObj _CONCAT(NAME,__LINE__)
// such that
//   LINEBASEDOBJ(Foo);
// gets expanded as
//   CLineBasedObj Foo28;
//

#define IS_PROPERTY_GLOBAL(type, propertyName, editorFlags); \
	RTTI::RTTI::Instance()->RegisterProperty(this, nullptr, propertyName, typeid(type).name(), editorFlags);

#define IS_PROPERTY(type, ptr, propertyName, editorFlags); \
	RTTI::RTTI::Instance()->RegisterProperty(this, (void*)(&ptr), propertyName, typeid(type).name(), editorFlags);

#define IS_REGISTER_PROPERTY(type, ptr, propertyName, editorFlags); \
	RTTI::RTTI::Instance()->RegisterProperty(this, (void*)(&ptr), propertyName, typeid(type).name(), editorFlags);

#define IS_GET_PROPERTY(ownerPtr, propertyName) \
	RTTI::RTTI::Instance()->GetProperty(ownerPtr, propertyName);

#define IS_GET_ALL_PROPERTIES(ownerPtr, edtiroFlags) \
	RTTI::RTTI::Instance()->GetAllProperties(ownerPtr, edtiroFlags);
