#pragma once

#include "Insight/Core.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/UUID.h"

namespace Insight
{
	namespace Library
	{
		template<typename T>
		class Library : public TSingleton<Library<T>>
		{
		public:
			Library() { SetInstancePtr(this); }
			~Library();

			T* AddAsset(const std::string& uuid, T* asset);
			T* AddAsset();
			T* GetAsset(const std::string& uuid);
			void RemoveAsset(const std::string& uuid);

		protected:
			bool Exists(const std::string& uuid);

			std::unordered_map<std::string, T*> m_assets;
		};

		template<typename T>
		inline Library<T>::~Library()
		{
			for (auto it = GetInstance()->m_assets.begin(); it != GetInstance()->m_assets.end(); ++it)
			{
				DELETE_ON_HEAP((*it).second);
			}
			GetInstance()->m_assets.clear();
			ClearPtr();
		}

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
		inline T* Library<T>::AddAsset()
		{
			T* t = NEW_ON_HEAP(T);
			Insight::UUID* uuid = static_cast<Insight::UUID*>(t);
			return AddAsset(uuid->GetUUID(), t);
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

namespace Insight
{
	namespace Render
	{
		class Shader;
	}
}

typedef Insight::Library::Library<Insight::Render::Shader> ShaderLibrary;