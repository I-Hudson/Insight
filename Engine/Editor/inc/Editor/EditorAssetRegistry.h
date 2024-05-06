#pragma once

#include "Asset/AssetRegistry.h"

namespace Insight::Editor
{
    class IS_EDITOR EditorAssetRegistry : public Runtime::AssetRegistry, public Core::Singleton<EditorAssetRegistry>
    {
    public:
        EditorAssetRegistry() = default;
        virtual ~EditorAssetRegistry() override = default;
    };
}