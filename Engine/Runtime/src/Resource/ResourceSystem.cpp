#include "Resource/ResourceSystem.h"
#include "Resource/ResourceManager.h"

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

        void ResourceSystem::Initialise()
        {
            m_database.Initialise();
            ResourceManagerExt::m_database = &m_database;
            m_state = Core::SystemStates::Initialised;
        }

        void ResourceSystem::Shutdown()
        {
            ResourceManagerExt::UnloadAll();
            ResourceManagerExt::m_database = nullptr;
            m_database.Shutdown();
            m_state = Core::SystemStates::Not_Initialised;
        }
    }
}