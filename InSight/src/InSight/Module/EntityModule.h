#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"

#include <vector>

class Entity;

namespace Insight
{
	namespace Module
	{
		class EntityModule : public TSingleton<EntityModule>, public Module
		{
		public:
			EntityModule(ModuleStartupData& startupData = ModuleStartupData());
			virtual ~EntityModule();

			virtual void Update(const float& deltaTime) override;

		private:
			Entity* Create(const std::string& id);
			void Delete(Entity* ptr);

			static std::vector<Entity*> m_entities;
			friend Entity;
		};
	}
}
