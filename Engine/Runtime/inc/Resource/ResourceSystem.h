#pragma once

#include "Core/ISysytem.h"

#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Runtime
    {
        class ResourceSystem : public Core::ISystem
        {
        public:
            ResourceSystem();
            virtual ~ResourceSystem() override;

            IS_SYSTEM(ResourceSystem);

            void Update(float const deltaTime);

            void ClearDatabase();

            // Begin - ISystem -
            virtual void Initialise() override;
            virtual void Shutdown() override;
            // End - ISystem -

        private:
            ResourceManager m_resourceMangaer;
        };
    }
}