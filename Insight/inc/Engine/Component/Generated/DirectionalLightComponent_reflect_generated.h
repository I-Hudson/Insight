 // This file is auto generated please don't modify.
#include "ReflectStructs.h"
#include "Core/Util.h"

#ifdef DirectionalLightComponent_reflect_generated_h
#error "DirectionalLightComponent_reflect_generated.h already included, missing 'pragma once' in DirectionalLightComponent.h"
#endif DirectionalLightComponent_reflect_generated_h
#define DirectionalLightComponent_reflect_generated_h

#define DirectionalLightComponent_Source_h_19_PROPERTIES \
private:\
	static Reflect::ReflectMemberProp __REFLECT_MEMBER_PROPS__[4];\


#define DirectionalLightComponent_Source_h_19_FUNCTION_DECLARE \
private:\


#define DirectionalLightComponent_Source_h_19_FUNCTION_GET \
public:\
	virtual Reflect::ReflectFunction GetFunction(const char* functionName) override;\


#define DirectionalLightComponent_Source_h_19_PROPERTIES_OFFSET \
private:\
	static int __REFLECT__FOV() { return offsetof(DirectionalLightComponentData, FOV); }; \
	static int __REFLECT__Direction() { return offsetof(DirectionalLightComponentData, Direction); }; \
	static int __REFLECT__NearPlane() { return offsetof(DirectionalLightComponentData, NearPlane); }; \
	static int __REFLECT__FarPlane() { return offsetof(DirectionalLightComponentData, FarPlane); }; \


#define DirectionalLightComponent_Source_h_19_PROPERTIES_GET \
public:\
virtual Reflect::ReflectMember GetMember(const char* memberName) override;\
virtual std::vector<Reflect::ReflectMember> GetMembers(std::vector<std::string> const& flags) override;\


#define DirectionalLightComponent_Source_h_19_GENERATED_BODY \
DirectionalLightComponent_Source_h_19_PROPERTIES \
DirectionalLightComponent_Source_h_19_FUNCTION_DECLARE \
DirectionalLightComponent_Source_h_19_FUNCTION_GET \
DirectionalLightComponent_Source_h_19_PROPERTIES_OFFSET \
DirectionalLightComponent_Source_h_19_PROPERTIES_GET \


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID DirectionalLightComponent_Source_h
