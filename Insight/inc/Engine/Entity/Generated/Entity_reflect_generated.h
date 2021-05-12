 // This file is auto generated please don't modify.
#include "ReflectStructs.h"
#include "Core/Util.h"

#ifdef Entity_reflect_generated_h
#error "Entity_reflect_generated.h already included, missing 'pragma once' in Entity.h"
#endif Entity_reflect_generated_h
#define Entity_reflect_generated_h

#define Entity_Source_h_14_PROPERTIES \
private:\
	static Reflect::ReflectMemberProp __REFLECT_MEMBER_PROPS__[1];\


#define Entity_Source_h_14_FUNCTION_DECLARE \
private:\


#define Entity_Source_h_14_FUNCTION_GET \
public:\
	virtual Reflect::ReflectFunction GetFunction(const char* functionName) override;\


#define Entity_Source_h_14_PROPERTIES_OFFSET \
private:\
	static int __REFLECT__ShowDebug() { return offsetof(EntityData, ShowDebug); }; \


#define Entity_Source_h_14_PROPERTIES_GET \
public:\
virtual Reflect::ReflectMember GetMember(const char* memberName) override;\
virtual std::vector<Reflect::ReflectMember> GetMembers(std::vector<std::string> const& flags) override;\


#define Entity_Source_h_14_GENERATED_BODY \
Entity_Source_h_14_PROPERTIES \
Entity_Source_h_14_FUNCTION_DECLARE \
Entity_Source_h_14_FUNCTION_GET \
Entity_Source_h_14_PROPERTIES_OFFSET \
Entity_Source_h_14_PROPERTIES_GET \


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Entity_Source_h
