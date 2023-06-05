#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "TypeDrawers.gen.h"

#include "ECS/Components/TagComponent.h"

#include "Core/Memory.h"

#include "Reflect.h"

#include <typeinfo>
#include <glm/glm.hpp>

namespace Insight
{
    namespace Editor
    {
        TypeDrawerRegister::TypeDrawerRegister()
        {
            RegisterAllTypeDrawers();
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

        std::string TypeDrawerRegister::GetTypeDrawerTypeName(const std::string& typeName) const
        {
            std::string typeNameMod = RemoveString(typeName, "class");
            typeNameMod = RemoveString(typeNameMod, "struct");
            return typeNameMod;
        }
    }
}