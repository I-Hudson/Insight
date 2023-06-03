#pragma once

#include "Editor/TypeDrawers/ITypeDrawer.h"

namespace Insight
{
    namespace Editor
    {
        class IS_EDITOR TypeDrawer_TagComponent : public ITypeDrawer
        {
        public:
            virtual void Draw(void* data) const override;

        private:
            std::unordered_map<void*, std::string> m_selectedIds;
            IS_REGISTER_TYPE_DRAWER_DEC(TypeDrawer_TagComponent);
        };
    }
}