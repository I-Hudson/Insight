#pragma once

#include "Insight/Core.h"

namespace Insight
{
	namespace Module
	{
		struct ModuleStartupData
		{
			bool ManuallUpdate = false;
		};

		class IS_API Module
		{
		public:
			Module() { }
			virtual ~Module() { }

			virtual void Startup(ModuleStartupData startupData = ModuleStartupData()) = 0;
			virtual void Shutdown() = 0;
			virtual void Update(const float& deltaTime) = 0;

			const bool ShouldManuallUpate() const { return m_manuallUpdate; }

			friend class ModuleManager;

		protected:
			bool m_manuallUpdate;
		};
	}
}