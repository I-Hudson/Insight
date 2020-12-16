#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"
#include "Insight/Templates/TSingleton.h"

namespace Insight
{
	namespace Library
	{
		template<typename T>
		class Library
		{
		public:
			Library() { }
			virtual ~Library();

			SharedPtr<T> AddAsset(const std::string& uuid, SharedPtr<T> asset);
			SharedPtr<T> AddAsset();
			SharedPtr<T> GetAsset(const std::string& uuid);
			void RemoveAsset(const std::string& uuid);

		protected:
			bool Exists(const std::string& uuid);

			std::unordered_map<std::string, SharedPtr<T>> m_assets;
		};

		template<typename T>
		inline Library<T>::~Library()
		{
			m_assets.clear();
		}

		template<typename T>
		inline SharedPtr<T> Library<T>::AddAsset(const std::string& uuid, SharedPtr<T> asset)
		{
			if (Exists(uuid))
			{
				return asset;
			}

			m_assets[uuid] = asset;
			return asset;
		}

		template<typename T>
		inline SharedPtr<T> Library<T>::AddAsset()
		{
			SharedPtr<T> t = CreateSharedPtr<T>();
			SharedPtr<T> uuid = StaticPointerCast<Insight::UUID>(t);
			return AddAsset(uuid->GetUUID(), t);
		}

		template<typename T>
		inline SharedPtr<T> Library<T>::GetAsset(const std::string& uuid)
		{
			if (Exists(uuid))
			{
				return m_assets[uuid];
			}
			return nullptr;
		}

		template<typename T>
		inline bool Library<T>::Exists(const std::string& uuid)
		{
			return m_assets.find(uuid) != m_assets.end();
		}
	}
}