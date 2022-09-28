#pragma once

#include "Core/TypeAlias.h"

#include <string>
#include <xhash>
#include <unordered_map>
#include <functional>

namespace Insight
{
	namespace Runtime
	{
		class Resource;

		/// @brief Define a unique resource type.
		class ResourceTypeId
		{
		public:
			ResourceTypeId() = delete;
			ResourceTypeId(const char* type_name);

			std::string GetTypeName() const;
			u64 GetHash() const;

			bool operator==(ResourceTypeId const& other) const;
			bool operator!=(ResourceTypeId const& other) const;

		private:
			std::string m_type_name;
			u64 m_hash = 0;
		};

		/// @brief Utility class for lookups to create a resource class from a ResourceTypeId.
		class ResourceTypeIdToResource
		{
			using CreateFunc = std::function<Resource*()>;
		public:
			static void RegisterResource(ResourceTypeId type_id, CreateFunc func);

			static Resource* CreateResource(ResourceTypeId);

		private:
			static std::unordered_map<ResourceTypeId, CreateFunc> m_map;
		};
	}
}

namespace std
{
	using ResourceTypeId = Insight::Runtime::ResourceTypeId;

	template<>
	struct hash<ResourceTypeId>
	{
		u64 operator()(ResourceTypeId const& other) const
		{
			return std::hash<std::string>()(other.GetTypeName());
		}
	};
}