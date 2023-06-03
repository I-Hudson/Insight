#pragma once

#include "Editor/Defines.h"
#include "Core/Singleton.h"
#include "Core/Memory.h"
#include "Core/Logger.h"

#include <unordered_map>

namespace Insight
{
    namespace Editor
    {
        class ITypeDrawer;
        class IS_EDITOR TypeDrawerRegister : public Core::Singleton<TypeDrawerRegister>
        {
        public:
            TypeDrawerRegister();
            ~TypeDrawerRegister();

            template<typename T>
            static void RegisterStaticDrawer(const char* typeName)
            {
                std::string typeNameMod = typeName;
                std::string_view stringsToRemove[] =
                {
                    "struct",
                    "class"
                };
                for (std::string_view view : stringsToRemove)
                {
                    if (u64 index = typeNameMod.find(view); index != std::string::npos)
                    {
                        typeNameMod = typeNameMod.substr(view.size() + 1);
                        break;
                    }
                }

                if (s_staticDrawersRegistered.find(typeNameMod) != s_staticDrawersRegistered.end())
                {
                    return;
                }
                s_staticDrawersRegistered[typeNameMod] = ::New<T>();
            }

            const ITypeDrawer* GetDrawer(const char* typeName) const;

        private:
            void MoveStaticDrawersToDrawers();

        private:
            static std::unordered_map<std::string, ITypeDrawer*> s_staticDrawersRegistered;
            std::unordered_map<std::string, ITypeDrawer*> m_drawers;
        };
    }
}