#pragma once

#include "Runtime/Defines.h"

#include "Core/TypeAlias.h"

#include "Serialisation/ISerialisable.h"
#include "Serialisation/Serialiser.h"

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
		class IS_RUNTIME ResourceTypeId : public Serialisation::ISerialisable
		{
		public:
			ResourceTypeId();
			ResourceTypeId(const char* type_name);

			IS_SERIALISABLE_H(ResourceTypeId)

			std::string GetTypeName() const;
			//u64 GetHash() const;

			bool operator==(ResourceTypeId const& other) const;
			bool operator!=(ResourceTypeId const& other) const;

		private:
			std::string m_type_name;
			u64 m_hash = 0;

			template<typename>
			friend struct ::Insight::Serialisation::SerialiserObject;
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

	OBJECT_SERIALISER(Runtime::ResourceTypeId, 2,
		SERIALISE_PROPERTY(std::string, m_type_name, 1, 0)
		SERIALISE_PROPERTY(u64, m_hash, 1, 2)
		);
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