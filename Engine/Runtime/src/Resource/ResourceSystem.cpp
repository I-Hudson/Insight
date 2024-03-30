#include "Resource/ResourceSystem.h"
#include "Resource/ResourceManager.h"

#include "Resource/Loaders/ResourceLoaderRegister.h"
#include "ResourceRegister.gen.h"

#include "Serialisation/Archive.h"

#include "Core/Profiler.h"

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
            m_resourceMangaer.Update(deltaTime);
        }

        void ResourceSystem::ClearDatabase()
        {
            m_resourceMangaer.ClearDatabase();
        }

        void ResourceSystem::Initialise()
        {
            IS_PROFILE_FUNCTION();
            ResourceLoaderRegister::Initialise();

            Engine::RegisterAllResources();

            m_resourceMangaer.Initialise();
            m_state = Core::SystemStates::Initialised;
        }

        void ResourceSystem::Shutdown()
        {
            IS_PROFILE_FUNCTION();

            m_resourceMangaer.Shutdown();

            ResourceLoaderRegister::Shutdown();
            m_state = Core::SystemStates::Not_Initialised;
        }
    }
}