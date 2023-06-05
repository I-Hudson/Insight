#pragma once

#include "Editor/Defines.h"
#include "Core/Singleton.h"
#include "Core/Memory.h"
#include "Core/Logger.h"
#include "Core/StringUtils.h"

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
            void RegisterTypeDrawer()
            {
                static_assert(std::is_base_of_v<ITypeDrawer, T>);
                std::string typeDrawerTypeName = T::GetTypeName();
                if (m_drawers.find(typeDrawerTypeName) != m_drawers.end())
                {
                    IS_CORE_WARN("[TypeDrawerRegister::RegisterDrawer] TypeDrawer already registed for '{}'.", typeDrawerTypeName);
                    return;
                }
                m_drawers[typeDrawerTypeName] = ::New<T>();
            }

            template<typename T>
            void UnregisterTypeDrawer()
            {
                static_assert(std::is_base_of_v<ITypeDrawer, T>);
                if (auto iter = m_drawers.find(T::GetTypeName());
                    iter != m_drawers.end())
                {
                    ::Delete(iter->second);
                    m_drawers.erase(iter);
                }
            }

            const ITypeDrawer* GetDrawer(const char* typeName) const;

        private:
            std::string GetTypeDrawerTypeName(const std::string& typeName) const;

        private:
            std::unordered_map<std::string, ITypeDrawer*> m_drawers;
        };
    }
}