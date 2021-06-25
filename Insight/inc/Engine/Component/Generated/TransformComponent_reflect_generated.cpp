#include "../TransformComponent.h"

Reflect::ReflectMemberProp TransformComponentData::__REFLECT_MEMBER_PROPS__[3] = {
	Reflect::ReflectMemberProp("Position", Reflect::Util::GetTypeName<glm::vec3>(), __REFLECT__Position(), {"ShowInEditor"}),
	Reflect::ReflectMemberProp("Rotation", Reflect::Util::GetTypeName<glm::quat>(), __REFLECT__Rotation(), {"ShowInEditor"}),
	Reflect::ReflectMemberProp("Scale", Reflect::Util::GetTypeName<glm::vec3>(), __REFLECT__Scale(), {"ShowInEditor"}),
};

Reflect::ReflectFunction TransformComponentData::GetFunction(const char* functionName)
{
	return __super::GetFunction(functionName);
}

Reflect::ReflectMember TransformComponentData::GetMember(const char* memberName)
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

std::vector<Reflect::ReflectMember> TransformComponentData::GetMembers(std::vector<std::string> const& flags)
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

