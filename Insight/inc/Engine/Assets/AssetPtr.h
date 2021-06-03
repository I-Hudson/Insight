#pragma once

#include "Engine/Core/Common.h"

namespace Insight::Module
{
	class AssetModule;
}

namespace Insight
{
	namespace Assets
	{
		class Asset;

		struct AssetPtrControlBlock
		{
			AssetPtrControlBlock()
			{
				RefCount.store(0, std::memory_order::memory_order_release);
			}
			std::atomic<u32> RefCount;

			void Incerment() { RefCount.fetch_add(1, std::memory_order::memory_order_release); }
			void Decerment() { RefCount.fetch_sub(1, std::memory_order::memory_order_release); }
		};
	}
	DECLARE_ENUM_7(AssetState, Loaded, Loading, Unloaded, Unloading, Missing, Croupt, NonLoaded)

	template<typename T>
	class AssetPtr
	{
	public:
		AssetPtr()
			: m_asset(nullptr), m_controlBlock(nullptr)
		{ }
		AssetPtr(T* asset, Assets::AssetPtrControlBlock* controlBlock)
			: m_asset(asset), m_controlBlock(controlBlock)
		{
			STATIC_ASSERT((std::is_base_of_v<Assets::Asset, T>), "[AssetPtr::AssetPtr] 'T' must be derived from 'Asset'.");
			ASSERT((m_asset && m_controlBlock) && "[AssetPtr::AssetPtr] 'asset' and 'controlBlock' must be valid pointers.");
			m_controlBlock->Incerment();
		}
		AssetPtr(const AssetPtr& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			m_controlBlock->Incerment();
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
			Clear();
		}

		AssetPtr operator=(const AssetPtr& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			m_controlBlock->Incerment();
			return *this;
		}
		AssetPtr operator=(AssetPtr&& other)
		{
			m_asset = other.m_asset;
			m_controlBlock = other.m_controlBlock;
			other.m_asset = nullptr;
			other.m_controlBlock = nullptr;
			return *this;
		}

		T& operator*() const noexcept { return *m_asset; }
		T* operator->() const noexcept { return m_asset; }

		template<typename T>
		AssetPtr<T> StaticCast()
		{
			T* ptr = static_cast<T*>(m_asset);
			return AssetPtr<T>(ptr, m_controlBlock);
		}
		template<typename T>
		AssetPtr<T> DynamicCastTo()
		{
			T* ptr = dynamic_cast<T*>(m_asset);
			if (ptr)
			{
				return AssetPtr<T>(ptr, m_controlBlock);
			}
			return AssetPtr<T>(nullptr, nullptr);
		}

		bool IsValid() const
		{
			if (m_controlBlock)
			{
				return m_controlBlock->RefCount.load(std::memory_order::memory_order_acquire) > 1;
			}
			return false;
		}
		AssetState GetState() const { return static_cast<Assets::Asset*>(m_asset)->GetState(); }
		void Clear() 
		{
			if (m_controlBlock)
			{
				m_controlBlock->Decerment();
			}
			m_asset = nullptr; 
			m_controlBlock = nullptr;
		}

	private:
		T* m_asset;
		Assets::AssetPtrControlBlock* m_controlBlock;
		friend Module::AssetModule;
	};
}