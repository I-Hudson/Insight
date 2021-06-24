#pragma once

#include "Engine/Platform/Platform.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"

namespace Insight::Core
{
	template<class T>
	class TSingleton
	{
	public:

		static T* Instance()
		{
			ASSERT(s_instance != nullptr && "[TSingleton::GetInstance] Usage of before SetInstancePtr is called!");
			return s_instance;
		}

		static bool IsInitialised()
		{
			return s_instance != nullptr;
		}

		TSingleton()
		{
			ASSERT(s_instance == nullptr && "[TSingleton::SetInstancePtr] Instance already created for.");
			s_instance = static_cast<T*>(this);
		}

		~TSingleton()
		{
			ASSERT(s_instance != nullptr && "[TSingleton::SetInstancePtr] Instance is already null.");
			s_instance = nullptr;
		}

	protected:
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

		static void ReleaseInstance()
		{
			s_instance = nullptr;
		}

		static T* s_instance;
	};

	template<class T>
	T* TSingleton<T>::s_instance;
}