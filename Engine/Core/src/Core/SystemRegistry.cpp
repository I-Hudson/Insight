#include "Core/SystemRegistry.h"
#include "Core/Assets.h"
#include "Algorithm/Vector.h"

namespace Insight
{
	namespace Core
	{
		SystemRegistry::SystemRegistry()
		{ }

		SystemRegistry::~SystemRegistry()
		{
			ASSERT(m_registeredSystems.empty());
		}

		void SystemRegistry::RegisterSystem(ISystem* system)
		{
			ASSERT(system != nullptr && !Algorithm::VectorContains(m_registeredSystems, system));
			m_registeredSystems.push_back(system);
		}

		void SystemRegistry::UnregisterSystem(ISystem* system)
		{
			auto iter = Algorithm::VectorFind(m_registeredSystems, system);
			ASSERT(iter != m_registeredSystems.end());
			m_registeredSystems.erase(iter);
		}

		void SystemRegistry::VerifyAllSystemsStates(SystemStates systemStateToVerify) const
		{
			for (auto const& system : m_registeredSystems)
			{
				ASSERT(system->GetSystemState() == systemStateToVerify);
			}
		}
	}
}