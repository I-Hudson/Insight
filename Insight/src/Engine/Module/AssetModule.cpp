#include "ispch.h"

#include "Engine/Module/AssetModule.h"
#include <filesystem>

namespace Insight::Module
{
	AssetModule::AssetModule()
	{ }

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
}