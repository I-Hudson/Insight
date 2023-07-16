#pragma once

#include "Resource/IResourceManager.h"

namespace Insight::Runtime
{
    class IS_RUNTIME ResourceManager : public IResourceManager, public Core::Singleton<ResourceManager>
    {
    public:
        ResourceManager() = default;
        virtual ~ResourceManager() override = default;
    };
}