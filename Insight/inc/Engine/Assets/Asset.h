#pragma once
#include "Engine/Core/Common.h"
#include <string>

namespace Insight::Module
{
	class AssetModule;
}

namespace Insight::Assets
{
	DECLARE_ENUM_7(AssetSate, Loaded, Loading, Unloaded, Unloading, Missing, Croupt, NonLoaded)

	const const char* AssetExtensions[] =
	{
		// Textures
		".png",
		".jpeg",
		//Models
		".fbx",
		".gltf",
		".obj",
	};

	class Asset
	{
	public:
		Asset(std::string path) { m_absolutePath = std::move(path); }
		AssetSate GetState() const { return m_state; }

	protected:
		virtual void Load() = 0;
		virtual void Unload() = 0;

	protected:
		std::string m_absolutePath;
		AssetSate m_state = AssetSate::NonLoaded;

		friend Module::AssetModule;
	};

	class FileAsset : public Asset
	{
	public:
		FileAsset(const std::string& path) : Asset(path) { }

	protected:
		virtual void Load() override { }
		virtual void Unload() override { }

	private:

	};
}