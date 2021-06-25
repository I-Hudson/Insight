 // This file is auto generated please don't modify.
#include "ReflectStructs.h"
#include "Core/Util.h"

#ifdef TransformComponent_reflect_generated_h
#error "TransformComponent_reflect_generated.h already included, missing 'pragma once' in TransformComponent.h"
#endif TransformComponent_reflect_generated_h
#define TransformComponent_reflect_generated_h

#define TransformComponent_Source_h_11_PROPERTIES \
private:\
	static Reflect::ReflectMemberProp __REFLECT_MEMBER_PROPS__[3];\


#define TransformComponent_Source_h_11_FUNCTION_DECLARE \
private:\


#define TransformComponent_Source_h_11_FUNCTION_GET \
public:\
	virtual Reflect::ReflectFunction GetFunction(const char* functionName) override;\


#define TransformComponent_Source_h_11_PROPERTIES_OFFSET \
private:\
	static int __REFLECT__Position() { return offsetof(TransformComponentData, Position); }; \
	static int __REFLECT__Rotation() { return offsetof(TransformComponentData, Rotation); }; \
	static int __REFLECT__Scale() { return offsetof(TransformComponentData, Scale); }; \


#define TransformComponent_Source_h_11_PROPERTIES_GET \
public:\
virtual Reflect::ReflectMember GetMember(const char* memberName) override;\
virtual std::vector<Reflect::ReflectMember> GetMembers(std::vector<std::string> const& flags) override;\


#define TransformComponent_Source_h_11_GENERATED_BODY \
TransformComponent_Source_h_11_PROPERTIES \
TransformComponent_Source_h_11_FUNCTION_DECLARE \
TransformComponent_Source_h_11_FUNCTION_GET \
TransformComponent_Source_h_11_PROPERTIES_OFFSET \
TransformComponent_Source_h_11_PROPERTIES_GET \


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID TransformComponent_Source_h
