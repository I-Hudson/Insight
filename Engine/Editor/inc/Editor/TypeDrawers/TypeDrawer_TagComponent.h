#pragma once

#include "Editor/TypeDrawers/ITypeDrawer.h"

#include "Editor/TypeDrawers/Generated/TypeDrawer_TagComponent_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class IS_EDITOR TypeDrawer_TagComponent : public ITypeDrawer
        {
            REFLECT_GENERATED_BODY()
        public:
            virtual void Draw(void* data) const override;

        private:
            std::unordered_map<void*, std::string> m_selectedIds;
        };
    }
}