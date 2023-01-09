#pragma once

#include "Core/ISysytem.h"

#include "Resource/ResourceDatabase.h"

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

            // Core::ISystem
            virtual void Initialise() override;
            virtual void Shutdown() override;

        private:
            ResourceDatabase m_database;
        };
    }
}