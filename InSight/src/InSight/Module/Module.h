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
			Module(ModuleStartupData& startupData = ModuleStartupData()) { m_manuallUpdate = startupData.ManuallUpdate; }
			virtual ~Module() { }

			virtual void Update(const float& deltaTime) = 0;

			const bool ShouldManuallUpate() const { return m_manuallUpdate; }

		protected:
			bool m_manuallUpdate;
		};
	}
}