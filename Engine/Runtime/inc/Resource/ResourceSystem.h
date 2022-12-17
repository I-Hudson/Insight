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

            virtual void Initialise() override;
            virtual void Shutdown() override;

        private:
            ResourceDatabase m_database;
        };
    }
}