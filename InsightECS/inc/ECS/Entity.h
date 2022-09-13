#pragma once

#include "Core/TypeAlias.h"
#include "Platform/Platform.h"
#include "ECS/Defines.h"
#include "Core/GUID.h"

#include "Core/Memory.h"

#include <string>
#include <vector>

#ifdef IS_ECS_ENABLED
#include <unordered_set>
#endif

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
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
#else
		class EntityManager;

		class IS_ECS Component
		{
		public:
			Component() { }
			~Component() { }

			// Called on creation.
			virtual void OnCreate() { }
			// Called on destruction.
			virtual void OnDestroy() { }

			// Called before first update (Only called once).
			virtual void OnBegin() { }
			// Called after last update (Only called once).
			virtual void OnEnd() { }

			// Called before any update.
			virtual void OnEarlyUpdate() { }
			// Called once per-frame 
			virtual void OnUpdate(const float delta_time) { }
			// Called after every udpate.
			virtual void OnLateUpdate() { }



		private:

		};

		class IS_ECS Entity
		{
		public:
			Entity()  = default;
			Entity(std::string name);

			Component* AddComponentByName(std::string_view component_type);
			Component* GetComponentByName(std::string_view component_type) const;
			void RemoveComponentByName(std::string_view component_type);

			Core::GUID GetGUID() const { return m_guid; }

			std::string GetName() const { return m_name; }
			void SetName(std::string entity_name) { m_name = std::move(m_name); }

		private:
			void Update(const float delta_time);

		private:
			Core::GUID m_guid = Core::GUID::s_InvalidGUID;
			std::string m_name;

			std::vector<RPtr<Component>> m_components;

			friend class EntityManager;
		};

#endif
	}
}