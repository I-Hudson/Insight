//#pragma once
//
//#include "Engine/Core/Core.h"
//#include "Engine/Module/Module.h"
//#include "Engine/Templates/TSingleton.h"
//#include "Engine/Library/ModelLibrary.h"
//#include "Engine/Library/ShaderLibrary.h"
//
//namespace Insight::Module
//{
//	class AssetModule : public Core::TSingleton<AssetModule>, public Module
//	{
//	public:
//		AssetModule();
//		virtual ~AssetModule();
//
//		virtual void Update(const float& deltaTime) override;
//
//		void Deserialize();
//
//	private:
//		bool m_deserlizaed;
//
//		//Library::ModelLibrary* m_modelLibrary;
//		//Library::ShaderLibrary* m_shaderLibrary;
//	};
//}