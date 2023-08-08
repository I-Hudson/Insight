#include "World/WorldSystem.h"

#include "Asset/AssetRegistry.h"

#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/FreeCameraControllerComponent.h"

#include "Serialisation/Archive.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "Algorithm/Vector.h"

#include "Core/Profiler.h"
#include "Core/Logger.h"

namespace Insight
{
    namespace Runtime
    {
        WorldSystem::WorldSystem()
        {
        }

        WorldSystem::~WorldSystem()
        { }

        void WorldSystem::Initialise()
        {
            IS_PROFILE_FUNCTION();

            TObjectPtr<World> defaultWorld = CreateWorld("DefaultWorld");
            Ptr<ECS::Entity> e = defaultWorld->AddEntity("MainCamera");
            e->AddComponentByName(ECS::CameraComponent::Type_Name);
            e->AddComponentByName(ECS::FreeCameraControllerComponent::Type_Name);

            AddActiveWorld(defaultWorld);
            m_state = Core::SystemStates::Initialised;
        }

        void WorldSystem::Shutdown()
        {
            IS_PROFILE_FUNCTION();

            for (auto& ptr : m_worlds)
            {
                ptr->Destroy();
            }
            m_worlds.clear();
            m_state = Core::SystemStates::Not_Initialised;
        }

        void WorldSystem::EarlyUpdate()
        {
            IS_PROFILE_FUNCTION();
            for (size_t sceneIdx = 0; sceneIdx < m_activeWorlds.size(); ++sceneIdx)
            {
                if (m_activeWorlds.at(sceneIdx))
                {
                    m_activeWorlds.at(sceneIdx)->EarlyUpdate();
                }
            }
        }

        void WorldSystem::Update(const float deltaTime)
        {
            IS_PROFILE_FUNCTION();
            for (size_t sceneIdx = 0; sceneIdx < m_activeWorlds.size(); ++sceneIdx)
            {
                if (m_activeWorlds.at(sceneIdx))
                {
                    m_activeWorlds.at(sceneIdx)->Update(deltaTime);
                }
            }
        }

        void WorldSystem::LateUpdate()
        {
            IS_PROFILE_FUNCTION();
            for (size_t sceneIdx = 0; sceneIdx < m_activeWorlds.size(); ++sceneIdx)
            {
                if (m_activeWorlds.at(sceneIdx))
                {
                    m_activeWorlds.at(sceneIdx)->LateUpdate();
                }
            }
        }

        TObjectPtr<World> WorldSystem::CreateWorld(std::string worldName, WorldTypes worldType)
        {
            TObjectOPtr<World> world = TObjectOPtr<World>(New<World, Core::MemoryAllocCategory::World>(worldName));
            world->m_worldName = worldName;
            world->m_worldType = worldType;
            world->Initialise();
            m_worlds.push_back(std::move(world));
            return m_worlds.back();
        }

        TObjectPtr<World> WorldSystem::CreatePersistentWorld(std::string worldName, WorldTypes worldType)
        {
            TObjectPtr<World> world = CreateWorld(worldName, worldType);
            world->m_persistentScene = true;
            AddActiveWorld(world);
            return world;
        }

        World* WorldSystem::LoadWorld(std::string_view filePath)
        {
            IS_PROFILE_FUNCTION();

            std::vector<Byte> worldData = AssetRegistry::Instance().LoadAsset(filePath);
            if (worldData.empty())
            {
                return nullptr;
            }

            Runtime::World::ResourceSerialiserType serialiser(true);
            if (!serialiser.Deserialise(worldData))
            {
                return nullptr;
            }

            TObjectPtr<World> world = CreateWorld();
            SetActiveWorld(world);
            world->Deserialise(&serialiser);
            return world.Get();
        }

        void WorldSystem::SetActiveWorld(TObjectPtr<World> world)
        {
            if (!world)
            {
                return;
            }

            std::vector<TObjectPtr<World>> worldsToRemove;
            for (size_t worldIdx = 0; worldIdx < m_activeWorlds.size(); ++worldIdx)
            {
                if (m_activeWorlds.at(worldIdx) && !m_activeWorlds.at(worldIdx)->IsPersistentScene())
                {
                    worldsToRemove.push_back(m_activeWorlds.at(worldIdx));
                }
            }

            // Remove all non persistent scenes
            for (size_t i = 0; i < worldsToRemove.size(); ++i)
            {
                RemoveWorld(worldsToRemove.at(i));
            }

            m_activeWorlds.push_back(world);
        }

        void WorldSystem::AddActiveWorld(TObjectPtr<World> world)
        {
            if (!world)
            {
                return;
            }

            if (std::find_if(m_activeWorlds.begin(), m_activeWorlds.end(), [&world](const TObjectPtr<World>& activeWorld)
                {
                    return activeWorld == world;
                }) == m_activeWorlds.end())
            {
                m_activeWorlds.push_back(world);
            }
        }

        void WorldSystem::RemoveWorld(TObjectPtr<World> world)
        {
            if (!world)
            {
                return;
            }

            bool foundWorld = Algorithm::VectorContains(m_worlds, world);
            if (!foundWorld)
            {
                return;
            }

            auto activeWorldIter = Algorithm::VectorFind(m_activeWorlds, world);
            if (activeWorldIter != m_activeWorlds.end())
            {
                m_activeWorlds.erase(activeWorldIter);
            }

            world->Destroy();
            auto worldIter = Algorithm::VectorFind(m_worlds, world);
            m_worlds.erase(worldIter);
        }

        TObjectPtr<World> WorldSystem::GetActiveWorld() const
        {
            for (size_t sceneIdx = 0; sceneIdx < m_activeWorlds.size(); ++sceneIdx)
            {
                if (m_activeWorlds.at(sceneIdx) && !m_activeWorlds.at(sceneIdx)->IsOnlySearchable())
                {
                    return m_activeWorlds.at(sceneIdx);
                }
            }
            IS_CORE_ERROR("[WorldSystem::GetActiveScene] No active scene found.");
            return TObjectPtr<World>();
        }

        TObjectPtr<World> WorldSystem::FindWorldByName(std::string_view sceneName)
        {
            for (auto const& world : m_worlds)
            {
                if (world->GetWorldName() == sceneName)
                {
                    return world;
                }
            }
            return TObjectPtr<World>();
        }

        TObjectPtr<World> WorldSystem::GetWorldFromIndex(u32 index) const
        {
            if (index < m_activeWorlds.size())
            {
                return m_activeWorlds.at(index);
            }
            return TObjectPtr<World>();
        }

        TObjectPtr<World> WorldSystem::GetWorldFromGuid(const Core::GUID& guid) const
        {
            IS_PROFILE_FUNCTION();

            for (const TObjectOwnPtr<World>& world : m_worlds)
            {
                if (world->GetGuid() == guid)
                {
                    return world;
                }
            }
            return nullptr;
        }

        std::vector<TObjectPtr<World>> WorldSystem::GetAllWorlds() const
        {
            IS_PROFILE_FUNCTION();

            std::vector<TObjectPtr<World>> worlds;
            for (size_t i = 0; i < m_activeWorlds.size(); ++i)
            {
                worlds.push_back(m_activeWorlds.at(i));
            }
            return worlds;
        }

        ECS::Entity* WorldSystem::GetEntityByGUID(const Core::GUID& guid) const
        {
            IS_PROFILE_FUNCTION();

            for (const TObjectOPtr<World>& world : m_worlds)
            {
                ECS::Entity* entity = world->GetEntityByGUID(guid);
                if (entity != nullptr)
                {
                    return entity;
                }
            }
            return nullptr;
        }
    }
}

#ifdef TESTING
#include "doctest.h"
#include "Core/Memory.h"
TEST_SUITE("SceneManagerSingleton")
{
    using namespace Insight::App;

    UPtr<WorldSystem> sceneManager;
    TEST_CASE("Make SceneManger UPtr")
    {
        sceneManager = MakeUPtr<WorldSystem>();
        CHECK(sceneManager.IsValid());
    }

    TEST_CASE("WorldSystem Singleton valid")
    {
        CHECK(sceneManager->IsValidInstance());
    }
}
#endif