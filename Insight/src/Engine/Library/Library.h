#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Templates/TSingleton.h"

	namespace Library
	{
		template<typename T>
		class Library
		{
		public:
			Library() { }
			virtual ~Library();

			T* AddAsset(const std::string& uuid, T* asset);
			T* GetAsset(const std::string& uuid);
			void RemoveAsset(const std::string& uuid);

		protected:
			bool Exists(const std::string& uuid);

			std::unordered_map<std::string, T*> m_assets;
		};

		template<typename T>
		inline Library<T>::~Library()
		{
			m_assets.clear();
		}

		template<typename T>
		inline T* Library<T>::AddAsset(const std::string& uuid, T* asset)
		{
			if (Exists(uuid))
			{
				return asset;
			}

			m_assets[uuid] = asset;
			return asset;
		}

		template<typename T>
		inline T* Library<T>::GetAsset(const std::string& uuid)
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