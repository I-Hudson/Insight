#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Assets/AssetPtr.h"
#include <string>

namespace Insight::Module
{
	class AssetModule;
}

namespace Insight::Assets
{
	struct AssetRegisty;
	DECLARE_ENUM_7(AssetSate, Loaded, Loading, Unloaded, Unloading, Missing, Croupt, NonLoaded)

	class Asset : public Object
	{
	public:
		AssetSate GetState() const { return m_state; }

	protected:
		virtual void LoadAsset(std::string path) = 0;
		virtual void UnloadAsset() = 0;

	protected:
		std::string m_absolutePath;
		AssetSate m_state = AssetSate::NonLoaded;

	private:
		friend Module::AssetModule;
	};

	/// <summary>
	/// Register an asset class with a single/multiple extensions
	/// </summary>
	//template<typename T>
	//struct AssetRegister
	//{
	//	AssetRegister(std::vector<std::string> extensions)
	//	{
	//		STATIC_ASSERT((std::is_base_of_v<Assets::Asset, T>), "[AssetRegister::AssetRegister] 'T' is not derived from 'Insight::Assets::Asset'.");
	//		for (auto& ex : extensions)
	//		{
	//			AssetRegisty::GetMap().emplace(ex, &AssetRegisty::CreateAsset<T>);
	//		}
	//	}
	//};

	struct AssetRegisty
	{
		using AssetRegisterMap = std::unordered_map<std::string, std::function<Asset*()>>;

		template<typename T>
		static void AddAssetToRegisty(std::vector<std::string> extensions)
		{
			STATIC_ASSERT((std::is_base_of_v<Assets::Asset, T>), "[AssetRegister::AssetRegister] 'T' is not derived from 'Insight::Assets::Asset'.");
			for (auto& ex : extensions)
			{
				AssetRegisty::GetMap().emplace(ex, &AssetRegisty::CreateAsset<T>);
			}
		}

		template<typename T>
		static T* CreateAsset()
		{
			STATIC_ASSERT((std::is_base_of_v<Assets::Asset, T>), "[AssetRegister::CreateAsset] 'T' is not derived from 'Insight::Assets::Asset'.");
			return ::New<T>();
		}

		static AssetRegisterMap& GetMap()
		{
			static AssetRegisterMap map;
			return map;
		}
	};

	class FileAsset : public Asset
	{
	public:
		FileAsset() { SetType<FileAsset>(); }
		~FileAsset() { }

	protected:
		virtual void LoadAsset(std::string absolutePath) override
		{
			m_absolutePath = std::move(absolutePath); 
			std::ifstream file(m_absolutePath);
			if (file.is_open())
			{
				std::string line;
				while (std::getline(file, line))
				{
					// using printf() in all tests for consistency
					m_fileData.insert(m_fileData.end(), line.begin(), line.end());
				}
				file.close();
			}
		}
		virtual void UnloadAsset() override { }

	private:
		std::vector<char> m_fileData;
	};

	//class ModelAsset : public Asset
	//{
	//public:
	//	ModelAsset();
	//	ModelAsset(const ModelAsset& other) { }
	//	ModelAsset(ModelAsset&& other) { }
	//	~ModelAsset();

	//	ModelAsset& operator=(const ModelAsset& other) { return *this; }
	//	ModelAsset& operator=(ModelAsset&& other) { return *this; }

	//protected:
	//	virtual void Load(std::string absolutePath) override;
	//	virtual void Unload() override { }
	//private:
	//	bool b = false;
	//};
}