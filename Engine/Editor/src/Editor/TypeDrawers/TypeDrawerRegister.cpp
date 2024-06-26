#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "TypeDrawers.gen.h"

#include "ECS/Components/TagComponent.h"

#include "Core/Memory.h"

#include "Reflect/Reflect.h"

#include <typeinfo>

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
            UnregisterAllTypeDrawers();
        }

        const ITypeDrawer* TypeDrawerRegister::GetDrawer(const char* typeName) const
        {
            if (auto iter = m_drawers.find(typeName);
                iter != m_drawers.end())
            {
                return iter->second;
            }

            for (auto&[name, drawer] : m_drawers)
            {
                if (name == typeName) 
                {
                    return drawer;
                }
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