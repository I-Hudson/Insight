#pragma once

#include "Runtime/Defines.h"

#include "Core/TypeAlias.h"

#include <string>
#include <xhash>
#include <unordered_map>
#include <functional>

namespace Insight
{
	namespace Runtime
	{
		class IResource;

		/// @brief Define a unique resource type.
		class IS_RUNTIME ResourceTypeId
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
		class IS_RUNTIME ResourceTypeIdToResource
		{
			using CreateFunc = std::function<IResource*()>;
		public:
			static void RegisterResource(ResourceTypeId type_id, CreateFunc func);

			static IResource* CreateResource(ResourceTypeId type_id);

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