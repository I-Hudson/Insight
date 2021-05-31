#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Assets/Asset.h"
#include "Engine/Assets/AssetPtr.h"

namespace Insight::Module
{
	class AssetModule : public Module, public Core::TSingleton<AssetModule>
	{
	public:
		AssetModule();
		virtual ~AssetModule();

		virtual void Update(const float& deltaTime) { }

		template<typename T>
		Assets::AssetPtr<T> Load(const std::string& path)
		{
			std::filesystem::path absPath = std::move(std::filesystem::absolute(std::filesystem::path(path)));
			if (!std::filesystem::exists(absPath))
			{
				IS_CORE_WARN("[AssetModule::Load] Trying to load non existent asset '{0}'.", path);
				return Assets::AssetPtr<T>(nullptr, nullptr);
			}

			std::string absPathString = std::move(absPath.u8string());
			auto itrAsset = m_assets.find(absPathString);
			auto itrControlBlock = m_controlBlocks.find(absPathString);
			if (itrAsset != m_assets.end())
			{
				return Assets::AssetPtr<T>(static_cast<T*>(itrAsset->second), itrControlBlock->second);
			}

			T* newAsset = ::New<T>(absPathString);
			Assets::AssetPtrControlBlock* newControlBlock = ::New<Assets::AssetPtrControlBlock>();
			static_cast<Assets::Asset*>(newAsset)->Load();

			m_assets.emplace(absPathString, newAsset);
			m_controlBlocks.emplace(absPathString, newControlBlock);

			return Assets::AssetPtr<T>(newAsset, newControlBlock);
		}

	private:


	private:
		std::unordered_map<std::string, Assets::Asset*> m_assets;
		std::unordered_map<std::string, Assets::AssetPtrControlBlock*> m_controlBlocks;
	};
}