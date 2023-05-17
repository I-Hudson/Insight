#pragma once

#include "Core/Singleton.h"

#include <unordered_map>

namespace Insight
{
    namespace Editor
    {
        class ITypeDrawer;
        class TypeDrawerRegister : public Core::Singleton<TypeDrawerRegister>
        {
        public:
            TypeDrawerRegister();
            ~TypeDrawerRegister();

            const ITypeDrawer* GetDrawer(const char* typeName) const;

        private:
            std::unordered_map<std::string, ITypeDrawer*> m_drawers;
        };
    }
}