#pragma once

#include "Runtime/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include "Serialisation/ISerialisable.h"
#include "Serialisation/Serialiser.h"

#include <string>
#include <unordered_map>
#include <functional>
#include <xhash>

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

			operator bool() const;
			bool operator==(ResourceTypeId const& other) const;
			bool operator!=(ResourceTypeId const& other) const;

		private:
			std::string m_type_name;
			u64 m_hash = 0;

			template<typename>
			friend struct ::Insight::Serialisation::SerialiserObject;
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

namespace Insight
{
	namespace Runtime
	{
		/// @brief Utility class for lookups to create a resource class from a ResourceTypeId.
		class IS_RUNTIME ResourceRegister
		{
			using CreateFunc = std::function<IResource*()>;
		public:
			template<typename T>
			static void RegisterResource()
			{
				ResourceTypeId typeId = T::GetStaticResourceTypeId();
				if (auto itr = m_map.find(typeId); itr != m_map.end())
				{
					IS_CORE_WARN("[ResourceTypeIdToResource::RegisterResource] Resource type is aleady registered '{}'.", typeId.GetTypeName());
					return;
				}
				m_map[typeId] = []() { return New<T, Core::MemoryAllocCategory::Resources>(); };
				s_resourceExtensionToResourceTypeId[T::GetStaticResourceFileExtension()] = typeId;
			}

			static ResourceTypeId GetResourceTypeIdFromExtension(std::string_view fileExtension);
			static ResourceTypeId GetResourceTypeIdFromExtension(const std::string& fileExtension);

			static IResource* CreateResource(ResourceTypeId type_id);

		private:
			static std::unordered_map<ResourceTypeId, CreateFunc> m_map;
			static std::unordered_map<std::string, ResourceTypeId> s_resourceExtensionToResourceTypeId;
		};
	}

	OBJECT_SERIALISER(Runtime::ResourceTypeId, 2,
		SERIALISE_PROPERTY(std::string, m_type_name, 1, 0)
		SERIALISE_PROPERTY(u64, m_hash, 1, 2)
		);
}