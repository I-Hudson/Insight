#pragma once

#include "Resource/ResourceManager.h"

namespace Insight::Editor
{
    class EditorResourceManager : public Runtime::IResourceManager, public Core::Singleton<EditorResourceManager>
    {
    public:

        EditorResourceManager() = default;
        virtual ~EditorResourceManager() override = default;
    };
}