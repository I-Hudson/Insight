#include "ispch.h"
#include "../Entity.h"

Reflect::ReflectMemberProp EntityData::__REFLECT_MEMBER_PROPS__[1] = {
	Reflect::ReflectMemberProp("ShowDebug", Reflect::Util::GetTypeName<bool>(), __REFLECT__ShowDebug(), {"EditorOnly"}),
};

Reflect::ReflectFunction EntityData::GetFunction(const char* functionName)
{
	return Reflect::ReflectFunction(nullptr, nullptr);
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
	return Reflect::ReflectMember("null", Reflect::Util::GetTypeName<void>(), nullptr);
}

std::vector<Reflect::ReflectMember> EntityData::GetMembers(std::vector<std::string> const& flags)
{
	std::vector<Reflect::ReflectMember> members;
	for(auto& member : __REFLECT_MEMBER_PROPS__)
	{
		if(member.ContainsProperty(flags))
		{
			members.push_back(Reflect::ReflectMember(member.Name, member.Type, ((char*)this) + member.Offset));
		}
	}
	return members;
}

