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
        TypeDrawerRegister::TypeDrawerRegister()
        {
            m_drawers[Reflect::Util::GetTypeName<glm::mat4>()] = New<TypeDrawer_GLMMat4>();
            m_drawers[ECS::TagComponent::Type_Name] = New<TypeDrawer_TagComponent>();
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
    }
}