#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Library/Library.h"
#include "Insight/Library/ModelLibrary.h"

#include "Insight/Assimp/Model.h"
#include "Insight/Renderer/Shader.h"

namespace Insight
{
	namespace Module
	{
		class IS_API AssetModule : public TSingleton<AssetModule>, public Module
		{
		public:
			AssetModule();
			virtual ~AssetModule();

			virtual void Update(const float& deltaTime) override;

		private:
			Insight::Library::ModelLibrary* m_modelLibrary;
			ShaderLibrary* m_shaderLibrary;
		};
	}
}