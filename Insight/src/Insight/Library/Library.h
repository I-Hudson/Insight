#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"

namespace Insight
{
	namespace Library
	{
		template<typename T>
		class Library : public TSingleton<Library<T>>
		{
		public:
			Library() { SetInstancePtr(this); }
			~Library() { ClearPtr(); }

			T* AddAsset(const std::string& uuid, T* asset);
			T* GetAsset(const std::string& uuid);
			void RemoveAsset(const std::string& uuid);

		private:
			bool Exists(const std::string& uuid);

			std::unordered_map<std::string, T*> m_assets;
		};

		template<typename T>
		inline T* Library<T>::AddAsset(const std::string& uuid, T* asset)
		{
			if (Exists(uuid))
			{
				return asset;
			}

			GetInstance()->m_assets[uuid] = asset;
			return asset;
		}

		template<typename T>
		inline T* Library<T>::GetAsset(const std::string& uuid)
		{
			if (Exists(uuid))
			{
				return GetInstance()->m_assets[uuid];
			}
			return nullptr;
		}

		template<typename T>
		inline bool Library<T>::Exists(const std::string& uuid)
		{
			return GetInstance()->m_assets.find(uuid) != GetInstance()->m_assets.end();
		}
	}
}