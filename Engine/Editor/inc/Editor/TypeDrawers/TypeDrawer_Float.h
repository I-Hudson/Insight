#pragma once

#include "Editor/TypeDrawers/ITypeDrawer.h"

#include "Generated/TypeDrawer_Float_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
            class IS_EDITOR TypeDrawer_Float : public ITypeDrawer
        {
            REFLECT_GENERATED_BODY()
        public:
            static std::string GetTypeName();
            virtual void Draw(void* data, const std::string_view label = "") const override;
        };
    }
}