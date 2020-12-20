#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Core/Log.h"

namespace Insight
{
    class Application;

    template<class T>
    class IS_API TSingleton
    {
    public:

        static T* Instance()
        {
            IS_CORE_ASSERT(s_instance != nullptr, "[TSingleton::GetInstance] Usage of before SetInstancePtr is called!");
            return s_instance;
        }

        static bool IsInitialised()
        {
            return s_instance != nullptr;
        }

    protected:
        static void SetInstancePtr(T* instance)
        {
            IS_CORE_ASSERT(s_instance == nullptr, "[TSingleton::SetInstancePtr] Instance already created for.");
            s_instance = instance;
        }

        static void ClearPtr()
        {
            IS_CORE_ASSERT(s_instance != nullptr, "[TSingleton::SetInstancePtr] Instance is already null.");
            s_instance = nullptr;
        }

    private:
        template<typename... Args>
        static T* Create(Args... args)
        {
            SetInstancePtr(new T(std::forward<Args>(args)...));

            return Instance();
        }

        template<typename... Args>
        static T* CreateWithoutMemoryManager(Args... args)
        {
            SetInstancePtr(new T(std::forward<Args>(args)...));

            return Instance();
        }

        static void Destroy()
        {
            delete s_instance;
        }

        static void DestroyWithoutMemoryManager()
        {
            delete s_instance;
        }

        static T* s_instance;

        friend Application;
    };
}

template<class T>
T* Insight::TSingleton<T>::s_instance;