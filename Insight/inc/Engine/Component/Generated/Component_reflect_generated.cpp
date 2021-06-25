#include "../Component.h"

Reflect::ReflectMemberProp ComponentData::__REFLECT_MEMBER_PROPS__[1] = {
	Reflect::ReflectMemberProp("AllowRemovable", Reflect::Util::GetTypeName<bool>(), __REFLECT__AllowRemovable(), {"Editor"}),
};

Reflect::ReflectFunction ComponentData::GetFunction(const char* functionName)
{
	return __super::GetFunction(functionName);
}

Reflect::ReflectMember ComponentData::GetMember(const char* memberName)
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

std::vector<Reflect::ReflectMember> ComponentData::GetMembers(std::vector<std::string> const& flags)
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

