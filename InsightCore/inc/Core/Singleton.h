#pragma once

#include "Platform/Platform.h"

namespace Insight
{
	namespace Core
	{
		template<typename T>
		class Singleton
		{
			using TPtr = T*;
			using TRef = T&;

		public:
			Singleton()
			{
				ASSERT(s_instance == nullptr);
				s_instance = static_cast<T*>(this);
				ASSERT(s_instance != nullptr);
			}

			virtual ~Singleton()
			{
				s_instance = nullptr;
			}

			Singleton(const Singleton& other) = delete; // Can't copy a singleton.
			Singleton(Singleton&& other) = delete; // Can't move a singleton.
			
			static TRef Instance() { ASSERT_MSG(s_instance != nullptr, "[Singleton::Instance] 's_instance' must be a valid pointer."); return *s_instance; }
			static bool IsValidInstance() { return s_instance != nullptr; }

		private:
			static TPtr s_instance;
		};

		template<typename T>
		T* Singleton<T>::s_instance = nullptr;
	}
}