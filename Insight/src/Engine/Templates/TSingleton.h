#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"

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

        TSingleton()
        {
            IS_CORE_ASSERT(s_instance == nullptr, "[TSingleton::SetInstancePtr] Instance already created for.");
            s_instance = static_cast<T*>(this);
        }

        ~TSingleton()
        {
            IS_CORE_ASSERT(s_instance != nullptr, "[TSingleton::SetInstancePtr] Instance is already null.");
            s_instance = nullptr;
        }

    private:
        template<typename... Args>
        static T* Create(Args... args)
        {
            new T(std::forward<Args>(args)...);

            return Instance();
        }

        template<typename... Args>
        static T* CreateWithoutMemoryManager(Args... args)
        {
            new T(std::forward<Args>(args)...);

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

template<class T>
T* TSingleton<T>::s_instance;
