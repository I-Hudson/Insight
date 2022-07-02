#pragma once

#include "Core/TypeAlias.h"
#include "Platform/Platform.h"
#include "ECS/Defines.h"
#include "Core/GUID.h"

#include <string>
#include <unordered_set>

namespace Insight
{
	namespace ECS
	{
		class EntityManager;
		class ECSWorld;

		/// <summary>
		/// Store all relevant data for an entity.
		/// This is not stored on the Entity as the 
		/// </summary>
		struct IS_ECS EntityData
		{
			std::string Name;
			Core::GUID GUID = Core::GUID::s_InvalidGUID;
			std::unordered_map<u64, std::unordered_set<int>> Components;
		};

		/// <summary>
		/// Entity is a simple handle for an entity. This class
		/// shouldn't hold/store any critical data.
		/// </summary>
		class IS_ECS Entity
		{
		public:

			void SetName(std::string name);

			std::string GetName() const;
			int GetId() const;
			Core::GUID GetGuid() const;

			bool IsVaild() const;

		private:
			int m_id = -1;
			EntityManager* m_entityManager = nullptr;
			ECSWorld* m_ecsWorld = nullptr;
			friend class EntityManager;
		};

		const Entity c_InvalidEntity = Entity();
	}
}