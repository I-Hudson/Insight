#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Templates/TSingleton.h"
#include "JobSystem.h"
#include "Engine/Assets/Asset.h"
#include <tuple>

namespace Insight::Module
{
	class AssetModule : public Module, public Core::TSingleton<AssetModule>
	{
	public:
		AssetModule();
		virtual ~AssetModule();

		virtual void Update(const float& deltaTime) { }

		AssetPtr<Assets::Asset> Load(const std::string& path);
		template<typename T>
		AssetPtr<T> Load(const std::string& path)
		{
			auto [asset, controlBlock, absolutePath] = CheckForAsset(path);
			if (asset && controlBlock)
			{
				return AssetPtr<T>(static_cast<T*>(asset), controlBlock);
			}

			T* newAsset = ::New<T>();
			Assets::AssetPtrControlBlock* newControlBlock = ::New<Assets::AssetPtrControlBlock>();
			static_cast<Assets::Asset*>(newAsset)->LoadAsset(absolutePath);

			{
				std::lock_guard<std::mutex> lock(m_lock);
				m_assets.emplace(absolutePath, newAsset);
				m_controlBlocks.emplace(absolutePath, newControlBlock);
			}

			return AssetPtr<T>(newAsset, newControlBlock);
		}

		JS::JobWithResultSharedPtr<AssetPtr<Assets::Asset>> LoadAsync(const std::string& path);
		template<typename T>
		JS::JobWithResultSharedPtr<AssetPtr<T>> LoadAsync(const std::string& path)
		{
			auto job = JS::JobSystemManager::Instance().CreateJob(JS::JobPriority::Normal, [this, path]()
			{
				return Load<T>(path);
			});
			JS::JobSystemManager::Instance().ScheduleJob(job);
			return job;
		}

		bool Unload(std::string path);
		template<typename T>
		bool Unload(AssetPtr<T>& assetPtr)
		{
			STATIC_ASSERT((std::is_base_of_v<Assets::Asset, T>), "[AssetModule::Unload] 'T' must be derived from 'Insight::Assets::Asset'.");
			std::string absolutePath = assetPtr->m_absolutePath;
			assetPtr.Clear();
			bool result = Unload(absolutePath);
			return result;
		}

	private:
		std::tuple<Assets::Asset*, Assets::AssetPtrControlBlock*, std::string> CheckForAsset(std::string path);
		Assets::Asset* GetAssetFromExtension(std::string extension);

	private:
		std::unordered_map<std::string, Assets::Asset*> m_assets;
		std::unordered_map<std::string, Assets::AssetPtrControlBlock*> m_controlBlocks;
		std::mutex m_lock;

		static std::unordered_map<std::string, std::function<Assets::Asset*()>> m_assetRegister;
	};
}