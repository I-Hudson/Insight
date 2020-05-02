#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"

#include <vector>

class Entity;

namespace Insight
{
	namespace Module
	{
		class EntityModule : public Module
		{
		public:
			EntityModule(ModuleStartupData& startupData = ModuleStartupData()) { m_manuallUpdate = startupData.ManuallUpdate; }
			virtual ~EntityModule();

			virtual void Update(const float& deltaTime) override;

		private:
			static std::vector<Entity*> m_entities;
			friend Entity;
		};
	}
}
