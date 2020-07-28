#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"

namespace Insight
{
	namespace Module
	{
		class AssetModule : public TSingleton<AssetModule>, public Module
		{
		public:
			AssetModule(ModuleStartupData& startupData = ModuleStartupData());
			virtual ~AssetModule();

			virtual void Update(const float& deltaTime) override;

		private:
		};
	}
}