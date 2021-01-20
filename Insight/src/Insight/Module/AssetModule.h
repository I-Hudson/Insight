#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Library/ModelLibrary.h"
#include "Insight/Library/ShaderLibrary.h"

	namespace Module
	{
		class IS_API AssetModule : public TSingleton<AssetModule>, public Module
		{
		public:
			AssetModule();
			virtual ~AssetModule();

			virtual void Update(const float& deltaTime) override;

			void Deserialize();

		private:
			bool m_deserlizaed;

			SharedPtr<Library::ModelLibrary> m_modelLibrary;
			//Library::ShaderLibrary* m_shaderLibrary;
		};
	}