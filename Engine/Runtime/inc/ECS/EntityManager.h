#pragma once

#include "ECS/Entity.h"
#include "ECS/ComponentHandle.h"

#include "Serialisation/Serialiser.h"

#include <vector>
#include <queue>
#include <shared_mutex>

namespace Insight
{
	namespace Runtime
	{
		class World;
	}

	namespace ECS
	{
		class ECSWorld;

#ifdef IS_ECS_ENABLED
		/// THREAD_SAFE

		class IS_RUNTIME EntityManager
		{
		public:
			EntityManager();
			EntityManager(ECSWorld* ecsWorld);

			Entity AddNewEntity();
			void RemoveEntity(Entity& entity);

			void AddComponentToEntity(Entity entity, ComponentHandle handle);
			void RemoveComponentFromEntity(Entity entity, ComponentHandle handle);
			bool EntityHasComponent(Entity entity, ComponentHandle handle) const;

			EntityData GetEntityData(Entity entity) const;
			EntityData& GetEntityData(Entity entity);

		private:
			ECSWorld* m_ecsWorld = nullptr;
			std::vector<EntityData> m_entities;
			std::queue<int> m_freeEntities;

			std::shared_mutex m_lock;
		};
#else

		class IS_RUNTIME EntityManager : public Serialisation::ISerialisable
		{
		public:
			EntityManager();
			EntityManager(const EntityManager& other) = delete;
			EntityManager(EntityManager&& other);

			EntityManager& operator=(EntityManager&& other);

#ifdef ECS_ENABLED
			EntityManager(ECSWorld* ecsWorld);
#else
			void SetWorld(Runtime::World* world);
#endif

			Entity* AddNewEntity();
			Entity* AddNewEntity(std::string entity_name);
			void RemoveEntity(Entity*& entity);

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void Destroy();

			Ptr<Entity> GetEntityByName(std::string_view entity_name) const;
			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;
			std::vector<Ptr<ECS::Entity>> GetAllEntities() const;
			ECS::Entity* GetEntityByGUID(Core::GUID guid) const;

			IS_SERIALISABLE_H(EntityManager);

		private:
			Entity* AddNewEntity(Core::GUID guid);
			Entity* AddNewEntity(std::string entity_name, Core::GUID guid);
			Component* AddComponentToEntity(Core::GUID entityGuid, Core::GUID componentGuid, std::string componentTypeName);

		private:
#ifdef ECS_ENABLED
			ECSWorld* m_ecsWorld = nullptr;
#else
			Runtime::World* m_world = nullptr;
#endif
			std::vector<UPtr<Entity>> m_entities;
			std::shared_mutex m_lock;
		};
#endif
	}

	namespace Serialisation
	{
		struct EntityManagerEntities1 {};
		template<>
		struct ComplexSerialiser<EntityManagerEntities1, std::vector<UPtr<ECS::Entity>>, ECS::EntityManager>
		{
			void operator()(std::vector<UPtr<ECS::Entity>>& entities, ECS::EntityManager* entityManager, ISerialiser* serialiser) const
			{
				ASSERT(serialiser);

				if (serialiser->IsReadMode())
				{
					ASSERT(entityManager);

					u64 entitiesCount;
					serialiser->Read("EntitiesGuidsArraySize", entitiesCount);

					entities.reserve(entitiesCount);

					serialiser->StartArray("EntitiesGuids", entitiesCount);
					// Create all entities and assign there guid.
					for (size_t i = 0; i < entitiesCount; ++i)
					{
						std::string guid;
						serialiser->Read("", guid);
						Core::GUID entityGuid;
						entityGuid.StringToGuid(guid);

						entityManager->AddNewEntity(guid);
					}
					serialiser->StopArray();

					// Then create all components.
					u64 componentCount = 0;
					serialiser->Read("ComponentGuidsArraySize", componentCount);
					serialiser->StartArray("ComponentGuids", componentCount);
					for (size_t componentIdx = 0; componentIdx < componentCount; ++componentIdx)
					{
						std::string entityGuidString;
						std::string componentGuidString;
						std::string componentTypeName;

						serialiser->StartObject("ComponentToEntity");
						serialiser->Read("EntityGuid", entityGuidString);
						serialiser->Read("ComponentGuid", componentGuidString);
						serialiser->Read("ComponentType", componentTypeName);
						serialiser->StopObject();

						Core::GUID entityGuid;
						entityGuid.StringToGuid(entityGuidString);
						Core::GUID componentGuid;
						componentGuid.StringToGuid(componentGuidString);

						entityManager->AddComponentToEntity(entityGuid, componentGuid, componentTypeName);
					}
					serialiser->StopArray();
				}
				else
				{
					u64 componentGuidCount = 0;
					// Save all entities guid's first.
					PropertySerialiser<Core::GUID> guidSerialiser;
					serialiser->StartArray("EntitiesGuids", entities.size());
					for (auto const& e : entities)
					{
						serialiser->Write("", guidSerialiser(e->GetGUID()));
						componentGuidCount += e->GetComponentCount();
					}
					serialiser->StopArray();

					// Save all components guid's second.
					serialiser->StartArray("ComponentGuids", componentGuidCount);
					for (auto const& e : entities)
					{
						for (u32 componentIdx = 0; componentIdx < e->GetComponentCount(); ++componentIdx)
						{
							ECS::Component* component = e->GetComponentByIndex(componentIdx);
							if (component == nullptr)
							{
								continue;
							}

							serialiser->StartObject("ComponentToEntity");
							serialiser->Write("EntityGuid", guidSerialiser(e->GetGUID()));
							serialiser->Write("ComponentGuid", guidSerialiser(component->GetGuid()));
							serialiser->Write("ComponentType", component->GetTypeName());
							serialiser->StopObject();
						}
					}
					serialiser->StopArray();

					// Finally save all entities and their data.
					serialiser->StartArray("Entities", entities.size());
					for (auto const& e : entities)
					{
						e->Serialise(serialiser);
					}
					serialiser->StopArray();
				}
			}
		};
	}

	OBJECT_SERIALISER(ECS::EntityManager, 1,
		SERIALISE_COMPLEX(Serialisation::EntityManagerEntities1, m_entities, 1, 0)
	);
}