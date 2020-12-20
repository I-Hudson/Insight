#pragma once

#include "Insight/Core/Core.h"
#include <vector>

namespace Insight
{
	namespace Module
	{
		class ModuleManager;

		enum ModulePriority
		{
			LOW,
			MEMDIUM,
			HIGH
		};

		class IS_API Module
		{
		public:
			virtual ~Module() 
			{
				for (auto it = m_dependentUpon.begin(); it != m_dependentUpon.end(); ++it)
				{
					(*it)->m_dependencies--;
				}
				m_dependentUpon.clear();
			}

			virtual void OnCreate() { }
			virtual void Update(const float& deltaTime) = 0;

			const bool ShouldManuallUpate() const { return m_manuallUpdate; }

			Module& AddDependency(SharedPtr<Module> module)
			{
				if (!DependencyExists(m_dependentUpon, module))
				{
					module->m_dependencies++;
					m_dependentUpon.push_back(module);
				}
				return *this;
			}

			Module& RemoveDependency(SharedPtr<Module> module)
			{
				if (DependencyExists(m_dependentUpon, module))
				{
					auto it = std::find(m_dependentUpon.begin(), m_dependentUpon.end(), module);
					if (it != m_dependentUpon.end())
					{
						module->m_dependencies--;
						m_dependentUpon.erase(it);
					}
				}
				return *this;
			}

			const int& GetDependenciesCount() const { return m_dependencies; }
			void SetManuallyUpdate(const bool& state) { m_manuallUpdate = state; }

		protected:
			bool m_manuallUpdate = false;
			ModulePriority m_modulePriority;
			std::vector<SharedPtr<Module>> m_dependentUpon;
			int m_dependencies = 0;

			friend Module;
			friend ModuleManager;
		
		private:
			bool DependencyExists(std::vector< SharedPtr<Module>> vec, SharedPtr<Module> module)
			{
				return std::find(vec.begin(), vec.end(), module) != vec.end();
			}
		};
	}
}