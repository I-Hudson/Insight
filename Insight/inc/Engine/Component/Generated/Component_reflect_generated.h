 // This file is auto generated please don't modify.
#include "ReflectStructs.h"
#include "Core/Util.h"

#ifdef Component_reflect_generated_h
#error "Component_reflect_generated.h already included, missing 'pragma once' in Component.h"
#endif Component_reflect_generated_h
#define Component_reflect_generated_h

#define Component_Source_h_21_PROPERTIES \
private:\
	static Reflect::ReflectMemberProp __REFLECT_MEMBER_PROPS__[1];\


#define Component_Source_h_21_FUNCTION_DECLARE \
private:\


#define Component_Source_h_21_FUNCTION_GET \
public:\
	virtual Reflect::ReflectFunction GetFunction(const char* functionName) override;\


#define Component_Source_h_21_PROPERTIES_OFFSET \
private:\
	static int __REFLECT__AllowRemovable() { return offsetof(ComponentData, AllowRemovable); }; \


#define Component_Source_h_21_PROPERTIES_GET \
public:\
virtual Reflect::ReflectMember GetMember(const char* memberName) override;\
virtual std::vector<Reflect::ReflectMember> GetMembers(std::vector<std::string> const& flags) override;\


#define Component_Source_h_21_GENERATED_BODY \
Component_Source_h_21_PROPERTIES \
Component_Source_h_21_FUNCTION_DECLARE \
Component_Source_h_21_FUNCTION_GET \
Component_Source_h_21_PROPERTIES_OFFSET \
Component_Source_h_21_PROPERTIES_GET \


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Component_Source_h
