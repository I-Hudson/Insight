#pragma once

#include "Editor/TypeDrawers/ITypeDrawer.h"

#include "Generated/TypeDrawer_GLMMat4_reflect_generated.h"
namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class IS_EDITOR TypeDrawer_GLMMat4 : public ITypeDrawer
        {
            REFLECT_GENERATED_BODY()
        public:
            static std::string GetTypeName();
            virtual void Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label = "") const override;
        };
    }
}