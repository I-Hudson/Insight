#include "Resource/ResourceSystem.h"
#include "Resource/ResourceManager.h"

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
            ResourceManager::s_database = &m_database;
            m_database.Initialise();
            m_state = Core::SystemStates::Initialised;
        }

        void ResourceSystem::Shutdown()
        {
            Archive archive("./ResouceDataBase", ArchiveModes::Write);
            //std::string data = m_database.Serialise();
            //archive.Write(data.data(), data.size());
            //archive.Close();

            ResourceManager::UnloadAll();
            ResourceManager::s_database = nullptr;
            m_database.Shutdown();
            m_state = Core::SystemStates::Not_Initialised;
        }
    }
}