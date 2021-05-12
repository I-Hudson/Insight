#pragma once

#include "Engine/Platform/Platform.h"
#include "Engine/Platform/CriticalSection.h"
#include "Engine/Graphics/GPUResource.h"

namespace Insight::Graphics
{
	class GPUResourceCollection
	{
	public:
		GPUResourceCollection(const u32& size)
			: m_collection(size)
		{ }

		~GPUResourceCollection()
		{ }

		u64 GetMemoryUsage() const;

		void OnDeviceDestroy();

		void DumpToLog() const;

		void Add(GPUResource* resource);

		void Remove(GPUResource* resource);

	private:

		CriticalSection m_lock;
		std::vector<GPUResource*> m_collection;
	};

	template<typename T, 
		typename std::enable_if_t<std::is_base_of<GPUResource*, T>::value>* = nullptr>
		class GPUItemCache : public TSingleton<GPUItemCache<T>>
	{
	public:
		GPUItemCache()
		{
		}

		~GPUItemCache()
		{
			ReleaseGPUAll();
		}

		void SetCreateFunc(std::function<T()> createTFunc)
		{
			m_createTFunc = createTFunc;
		}

		bool GetItem(u64 hash, T& value)
		{
			ASSERT(m_createTFunc && "[GPUItemCache::GetItem<T>] A create callback must be set for the cache.");

			auto itr = m_collection.find(hash);
			if (itr != m_collection.end())
			{
				value = itr->second;
				return false;
			}

			T newItem = m_createTFunc();
			m_collection[hash] = newItem;
			value = newItem;
			return true;
		}

		void ReleaseGPUAll()
		{
			for (auto& kvp : m_collection)
			{
				GPUResource* resource = (GPUResource*)(kvp.second);
				resource->ReleaseGPU();
				::Delete(resource);
			}
			m_collection.clear();
		}

	private:
		std::unordered_map<u64, T> m_collection;
		std::function<T()> m_createTFunc;
	};
}
