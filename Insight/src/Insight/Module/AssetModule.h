#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Library/Library.h"

#include "Insight/Assimp/Model.h"
#include "Insight/Renderer/Shader.h"

namespace Insight
{
	namespace Module
	{
		class IS_API AssetModule : public TSingleton<AssetModule>, public Module
		{
		public:
			AssetModule(ModuleStartupData& startupData = ModuleStartupData());
			virtual ~AssetModule();

			virtual void Update(const float& deltaTime) override;

		private:
			ModelLibrary* m_modelLibrary;
			ShaderLibrary* m_shaderLibrary;
		};
	}
}