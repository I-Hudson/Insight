#include "Resource/ResourceTypeId.h"

namespace Insight
{
	namespace Runtime
	{
		ResourceTypeId::ResourceTypeId(const char* type_name)
			: m_type_name(type_name)
			, m_hash(std::hash<ResourceTypeId>()(*this))
		{
		}

		std::string ResourceTypeId::GetTypeName() const
		{
			return m_type_name;
		}

		u64 ResourceTypeId::GetHash() const
		{
			return m_hash;
		}

		bool ResourceTypeId::operator==(ResourceTypeId const& other) const
		{
			return m_hash == other.m_hash;
		}

		bool ResourceTypeId::operator!=(ResourceTypeId const& other) const
		{
			return !(*this == other);
		}
	}
}