#pragma once

#include "Core/Defines.h"
#include "Core/ISysytem.h"

#include "Platform/Platform.h"

#include <vector>

namespace Insight
{
	namespace Core
	{
		/// <summary>
		/// Store all the current registered systems.
		/// This does not update 
		/// </summary>
		class IS_CORE SystemRegistry
		{
		public:
			SystemRegistry();
			SystemRegistry(const SystemRegistry& other) = delete;
			~SystemRegistry();

			void RegisterSystem(ISystem* system);
			void UnregisterSystem(ISystem* system);

			void VerifyAllSystemsStates(SystemStates systemStateToVerify) const;

			template<typename T>
			INLINE T* GetSystem() const
			{
				static_assert(std::is_base_of<ISystem, T>::value, "[SystemRegistry::GetSystem] T is not derived from ISystem");
				for (auto pSystem : m_registeredSystems)
				{
					if (pSystem->GetSystemId() == T::s_SystemId)
					{
						return reinterpret_cast<T*>(pSystem);
					}
				}
				FAIL_ASSERT();
				return nullptr;
			}

		private:
			std::vector<ISystem*> m_registeredSystems;
		};
	}
}