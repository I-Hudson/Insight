#pragma once

#include "Core/Singleton.h"
#include "Core/ISysytem.h"
#include "Core/Memory.h"
#include "World/World.h"

#include <vector>

namespace Insight
{
	namespace Runtime
	{
		class IS_RUNTIME WorldSystem : public Core::Singleton<WorldSystem>, public Core::ISystem
		{
		public:
			WorldSystem();
			virtual ~WorldSystem() override;

			IS_SYSTEM(WorldSystem);

			virtual void Initialise() override;
			virtual void Shutdown() override;

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			TObjectPtr<World> CreateWorld(std::string worldName = "", WorldTypes worldType = WorldTypes::Game);
			// Create a new world which can not be unloaded. This world will alway be loaded.
			TObjectPtr<World> CreatePersistentWorld(std::string worldName = "", WorldTypes worldType = WorldTypes::Game);

			World* LoadWorld(std::string_view filePath);

			// Set a single scene as active.
			void SetActiveWorld(TObjectPtr<World> world);
			// Add a scene to be active.
			void AddActiveWorld(TObjectPtr<World> world);
			// Remove a scene from being active.
			void RemoveWorld(TObjectPtr<World> world);

			TObjectPtr<World> GetActiveWorld() const;
			TObjectPtr<World> FindWorldByName(std::string_view sceneName);
			TObjectPtr<World> GetWorldFromIndex(u32 index) const;
			TObjectPtr<World> GetWorldFromGuid(const Core::GUID& guid) const;

			std::vector<TObjectPtr<World>> GetAllWorlds() const;

			ECS::Entity* GetEntityByGUID(const Core::GUID& guid) const;

		private:
			std::vector<TObjectPtr<World>> m_activeWorlds;
			std::vector<TObjectOPtr<World>> m_worlds;
		};
	}
}