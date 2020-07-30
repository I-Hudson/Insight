#pragma once

#include "Insight/Core.h"
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
			virtual ~Module() { }

			virtual void Update(const float& deltaTime) = 0;

			const bool ShouldManuallUpate() const { return m_manuallUpdate; }

			Module& AddDependency(Module* module)
			{
				module->AddDependencyInternal(this);
				return *this;
			}

			Module& RemoveDependency(Module* module)
			{
				module->AddDependencyInternal(this);
				return *this;
			}

			const int& GetDependenciesCount() const { return m_dependencies.size(); }

			void SetManuallyUpdate(const bool& state) { m_manuallUpdate = state; }

		protected:
			bool m_manuallUpdate;
			ModulePriority m_modulePriority;
			std::vector<Module*> m_dependencies;

			friend Module;
			friend ModuleManager;
		
		private:
			void AddDependencyInternal(Module* module)
			{
				if (!DependencyExists(module))
				{
					m_dependencies.push_back(module);
				}
			}
			void RemoveDependencyInternal(Module* module)
			{
				if (DependencyExists(module))
				{
					auto it = std::find(m_dependencies.begin(), m_dependencies.end(), module);
					m_dependencies.erase(it);
				}
			}

			bool DependencyExists(Module* module)
			{
				return std::find(m_dependencies.begin(), m_dependencies.end(), module) != m_dependencies.end();
			}
		};
	}
}