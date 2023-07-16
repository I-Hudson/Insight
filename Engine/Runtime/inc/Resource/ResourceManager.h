#pragma once

#include "Resource/IResourceManager.h"

namespace Insight::Runtime
{
    class ResourceManager : public IResourceManager, public Core::Singleton<ResourceManager>
    {
    public:
        ResourceManager() = default;
        virtual ~ResourceManager() override = default;
    };
}