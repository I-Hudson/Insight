#pragma once

#include "Editor/TypeDrawers/ITypeDrawer.h"

namespace Insight
{
    namespace Editor
    {
        REFLECT_CLASS()
        class TypeDrawer_TagComponent : public ITypeDrawer
        {
        public:
            virtual void Draw(void* data) const override;

        private:
            std::unordered_map<void*, std::string> m_selectedIds;
        };
    }
}