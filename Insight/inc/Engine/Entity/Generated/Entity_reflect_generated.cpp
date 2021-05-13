#include "ispch.h"
#include "../Entity.h"

Reflect::ReflectMemberProp EntityData::__REFLECT_MEMBER_PROPS__[1] = {
	Reflect::ReflectMemberProp("ShowDebug", Reflect::Util::GetTypeName<bool>(), __REFLECT__ShowDebug(), {"EditorOnly"}),
};

Reflect::ReflectFunction EntityData::GetFunction(const char* functionName)
{
	return __super::GetFunction(functionName);
}

Reflect::ReflectMember EntityData::GetMember(const char* memberName)
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

std::vector<Reflect::ReflectMember> EntityData::GetMembers(std::vector<std::string> const& flags)
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

