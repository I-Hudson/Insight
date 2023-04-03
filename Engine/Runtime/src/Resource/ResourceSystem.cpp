#include "Resource/ResourceSystem.h"
#include "Resource/ResourceManager.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"

#include "Serialisation/Archive.h"

namespace Insight
{
    namespace Runtime
    {
        ResourceSystem::ResourceSystem()
        {
        }

        ResourceSystem::~ResourceSystem()
        {
        }

        void ResourceSystem::Update(float const deltaTime)
        {
            ResourceManager::Update(deltaTime);
        }

        void ResourceSystem::Initialise()
        {
            ResourceLoaderRegister::Initialise();

            ResourceManager::s_database = &m_database;
            m_database.Initialise();
            m_state = Core::SystemStates::Initialised;
        }

        void ResourceSystem::Shutdown()
        {
            ResourceManager::Shutdown();

            m_database.Shutdown();
            ResourceManager::s_database = nullptr;

            ResourceLoaderRegister::Shutdown();
            m_state = Core::SystemStates::Not_Initialised;
        }

        ResourceDatabase& ResourceSystem::GetDatabase()
        {
            return m_database;
        }

        const ResourceDatabase& ResourceSystem::GetDatabase() const
        {
            return m_database;
        }
    }
}