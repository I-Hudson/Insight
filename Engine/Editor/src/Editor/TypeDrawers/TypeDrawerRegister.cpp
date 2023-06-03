#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "Editor/TypeDrawers/TypeDrawer_GLMMat4.h"
#include "Editor/TypeDrawers/TypeDrawer_TagComponent.h"

#include "ECS/Components/TagComponent.h"

#include "Core/Memory.h"

#include "Reflect.h"

#include <typeinfo>
#include <glm/glm.hpp>

namespace Insight
{
    namespace Editor
    {
        std::unordered_map<std::string, ITypeDrawer*> TypeDrawerRegister::s_staticDrawersRegistered;

        TypeDrawerRegister::TypeDrawerRegister()
        {
            MoveStaticDrawersToDrawers();
        }

        TypeDrawerRegister::~TypeDrawerRegister()
        {
            for (auto& pair : m_drawers)
            {
                Delete(pair.second);
            }
            m_drawers.clear();
        }

        const ITypeDrawer* TypeDrawerRegister::GetDrawer(const char* typeName) const
        {
            if (auto iter = m_drawers.find(typeName);
                iter != m_drawers.end())
            {
                return iter->second;
            }
            return nullptr;
        }

        void TypeDrawerRegister::MoveStaticDrawersToDrawers()
        {
            for (auto& [TypeName, Drawer] : s_staticDrawersRegistered)
            {
                if (m_drawers.find(TypeName) != m_drawers.end())
                {
                    IS_CORE_WARN("[TypeDrawerRegister::MoveStaticDrawersToDrawers] Type name: '{}' already has a drawer registered.", TypeName);
                    return;
                }
                m_drawers[TypeName] = std::move(Drawer);
            }
            s_staticDrawersRegistered.clear();
        }
    }
}