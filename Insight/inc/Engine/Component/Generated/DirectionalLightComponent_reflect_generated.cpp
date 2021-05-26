#include "ispch.h"
#include "../DirectionalLightComponent.h"

Reflect::ReflectMemberProp DirectionalLightComponentData::__REFLECT_MEMBER_PROPS__[4] = {
	Reflect::ReflectMemberProp("FOV", Reflect::Util::GetTypeName<float>(), __REFLECT__FOV(), {"ShowInEditor"}),
	Reflect::ReflectMemberProp("Direction", Reflect::Util::GetTypeName<glm::vec3>(), __REFLECT__Direction(), {"ShowInEditor"}),
	Reflect::ReflectMemberProp("NearPlane", Reflect::Util::GetTypeName<float>(), __REFLECT__NearPlane(), {"ShowInEditor"}),
	Reflect::ReflectMemberProp("FarPlane", Reflect::Util::GetTypeName<float>(), __REFLECT__FarPlane(), {"ShowInEditor"}),
};

Reflect::ReflectFunction DirectionalLightComponentData::GetFunction(const char* functionName)
{
	return __super::GetFunction(functionName);
}

Reflect::ReflectMember DirectionalLightComponentData::GetMember(const char* memberName)
{
	for(const auto& member : __REFLECT_MEMBER_PROPS__)
	{
		if(memberName == member.Name)
		{
			//CheckFlags
			return Reflect::ReflectMember(member.Name, member.Type, ((char*)this) + member.Offset);
		}
	}
	return __super::GetMember(memberName);
}

std::vector<Reflect::ReflectMember> DirectionalLightComponentData::GetMembers(std::vector<std::string> const& flags)
{
	std::vector<Reflect::ReflectMember> members = __super::GetMembers(flags);
	for(auto& member : __REFLECT_MEMBER_PROPS__)
	{
		if(member.ContainsProperty(flags))
		{
			members.push_back(Reflect::ReflectMember(member.Name, member.Type, ((char*)this) + member.Offset));
		}
	}
	return members;
}

