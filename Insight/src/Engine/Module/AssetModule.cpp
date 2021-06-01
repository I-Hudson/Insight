#include "ispch.h"

#include "Engine/Module/AssetModule.h"
#include <filesystem>

namespace Insight::Module
{
	std::unordered_map<std::string, std::function<Assets::Asset* ()>> AssetModule::m_assetRegister;

	AssetModule::AssetModule()
	{
		Assets::AssetRegisty::AddAssetToRegisty<Assets::FileAsset>({ ".txt" });
		Assets::AssetRegisty::AddAssetToRegisty<Assets::FileAsset>({ ".txt" });
	}

	AssetModule::~AssetModule()
	{
		for (auto& asset : m_assets)
		{
			::Delete(asset.second);
		}

		for (auto& controlBlock : m_controlBlocks)
		{
			ASSERT(controlBlock.second->RefCount.load(std::memory_order::memory_order_acquire) == 0 &&
				   "[AssetModule::~AssetModule] Asset has a reference somewhere.");
			::Delete(controlBlock.second);
		}
	}

	Assets::AssetPtr<Assets::Asset> AssetModule::Load(const std::string& path)
	{
		auto [asset, controlBlock, absolutePath] = CheckForAsset(path);
		if (asset && controlBlock)
		{
			return Assets::AssetPtr<Assets::Asset>(asset, controlBlock);
		}

		std::string extension = std::move(std::filesystem::path(path).extension().u8string());
		Assets::Asset* newAsset = GetAssetFromExtension(extension);
		if (!newAsset)
		{
			return Assets::AssetPtr<Assets::Asset>(nullptr, nullptr);
		}

		Assets::AssetPtrControlBlock* newControlBlock = ::New<Assets::AssetPtrControlBlock>();
		static_cast<Assets::Asset*>(newAsset)->Load(absolutePath);

		{
			std::lock_guard<std::mutex> lock(m_lock);
			m_assets.emplace(absolutePath, newAsset);
			m_controlBlocks.emplace(absolutePath, newControlBlock);
		}

		return Assets::AssetPtr<Assets::Asset>(newAsset, newControlBlock);
	}

	JS::JobWithResultSharedPtr<Assets::AssetPtr<Assets::Asset>> AssetModule::LoadAsync(const std::string& path)
	{
		return JS::JobSystemManager::Instance().CreateJob(JS::JobPriority::Normal, [this, &path]()
		{
			return Load(path);
		});
	}

	bool AssetModule::Unload(std::string path)
	{
		auto [asset, controlBlock, absolutePath] = CheckForAsset(path);
		if (!asset && !controlBlock)
		{
			return true;
		}

		//ASSERT(controlBlock->RefCount.load(std::memory_order::memory_order_acquire) != 0 && "[AssetModule::Unload] Asset has no references. Should have at least one from 'AssetModule'.");
		if (controlBlock->RefCount.load(std::memory_order::memory_order_acquire) > 0)
		{
			IS_CORE_WARN("[AssetModule::Unload] Asset has a reference when trying to Unload. Asset: '{0}' Path: '{1}', not unloaded. Refs: '{2}'.", 
						 asset->GetType().GetTypeName(), absolutePath, controlBlock->RefCount.load());
			return false;
		}

		{
			std::lock_guard<std::mutex> lock(m_lock);
			m_assets.erase(absolutePath);
			m_controlBlocks.erase(absolutePath);
		}

		u32 assetSize = sizeof(*asset);
		::Delete(asset);
		::Delete(controlBlock);

		Platform::MemSet(asset, 0, assetSize);
		Platform::MemSet(controlBlock, 0, sizeof(Assets::AssetPtrControlBlock));
		return true;
	}

	std::tuple<Assets::Asset*, Assets::AssetPtrControlBlock*, std::string> AssetModule::CheckForAsset(std::string path)
	{
		std::filesystem::path absPath = std::move(std::filesystem::absolute(std::filesystem::path(path)));
		std::string absPathString = std::move(absPath.u8string());
		if (!std::filesystem::exists(absPath))
		{
			IS_CORE_WARN("[AssetModule::Load] Trying to load non existent asset '{0}'.", path);
			return { nullptr, nullptr, absPathString };
		}

		{
			std::lock_guard<std::mutex> lock(m_lock);
			auto itrAsset = m_assets.find(absPathString);
			auto itrControlBlock = m_controlBlocks.find(absPathString);
			if (itrAsset != m_assets.end())
			{
				return { itrAsset->second, itrControlBlock->second, absPathString };
			}
		}
		return { nullptr, nullptr, absPathString };
	}

	Assets::Asset* AssetModule::GetAssetFromExtension(std::string extension)
	{
		auto itr = Assets::AssetRegisty::GetMap().find(extension);
		if (itr != Assets::AssetRegisty::GetMap().end())
		{
			return itr->second();
		}
		return nullptr;
	}
}