#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Assets/Asset.h"

namespace Insight::Module
{
	class AssetModule;
}

namespace Insight::Assets
{
	DECLARE_ENUM_7(AssetState, Loaded, Loading, Unloaded, Unloading, Missing, Croupt, NonLoaded)
	struct AssetPtrControlBlock
	{
		AssetPtrControlBlock()
		{ RefCount.store(0, std::memory_order::memory_order_release); }
		std::atomic<u32> RefCount;
	};

	template<typename T>
	class AssetPtr
	{
	public:
		AssetPtr(const AssetPtr& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			m_controlBlock->RefCount.fetch_add(1, std::memory_order::memory_order_release);
		}
		AssetPtr(AssetPtr&& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			other.m_asset = nullptr;
			other.m_controlBlock = nullptr;
		}

		~AssetPtr()
		{
			if (m_controlBlock)
			{
				m_controlBlock->RefCount.fetch_sub(1, std::memory_order::memory_order_release);
			}
		}

		AssetPtr operator=(const AssetPtr& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			m_controlBlock->RefCount.fetch_add(1, std::memory_order::memory_order_release);
		}
		AssetPtr operator=(AssetPtr&& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			other.m_asset = nullptr;
			other.m_controlBlock = nullptr;
		}

		T* operator*() { return m_asset; }
		bool IsValid() const { return m_asset && m_controlBlock; }
		AssetState GetState() const { return static_cast<Asset*>(m_asset)->GetState(); }
		void Clear() 
		{
			m_asset = nullptr; 
			m_controlBlock->RefCount.fetch_sub(1, std::memory_order::memory_order_release);
			m_controlBlock = nullptr;
		}

	private:
		AssetPtr() = delete;
		AssetPtr(T* asset, AssetPtrControlBlock* controlBlock)
			:m_asset(asset), m_controlBlock(controlBlock)
		{ 
			STATIC_ASSERT((std::is_base_of_v<Asset, T>), "[AssetPtr::AssetPtr] 'T' must be derived from 'Asset'.");
			ASSERT((m_asset && m_controlBlock) && "[AssetPtr::AssetPtr] 'asset' and 'controlBlock' must be valid pointers.");
			m_controlBlock->RefCount.fetch_add(1, std::memory_order::memory_order_release);
		}

	private:
		T* m_asset;
		AssetPtrControlBlock* m_controlBlock;

		friend Module::AssetModule;
	};
}