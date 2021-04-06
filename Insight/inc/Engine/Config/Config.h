#pragma once

#include "Engine/Core/Common.h"
#include "ConfigRegistry.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Module/WindowModule.h"
#include "Engine/Module/GraphicsModule.h"

#include <string>

#define CONFIG_GET_VAL(expr) expr.GetVal()
#define CONFIG_VAL(expr) Config::GetInstance().expr.GetVal()
#define CONFIG_SET(key, val) Config::GetInstance().SetVal(key, val)
#define CONFIG_SET_P(expr, val) Config::GetInstance().expr.SetVal(val)

class IS_API Config
{
public:
	void Parse(const std::string& filePath);
	void ParseInFolder(const std::string& folderPath);
	void SetVal(const std::string& key, const std::string_view& val);

	static Config& GetInstance()
	{
		static Config instnace;
		return instnace;
	}

	Memory::MemoryManager::MemoryConfig MemoryConfig;
	Module::WindowModule::WindowConfig WindowConfig;
	Module::GraphicsModule::GraphicsConfig GraphicsConfig;
	//Renderer::RendererConfig RendererConfig;

private:
	const std::string GetKey(const std::string& line, const int delimiterPos);
	const std::string GetVal(const std::string& line, const int delimiterPos);
	void SetConfigVal(const std::string& key, const std::string_view& val);

	ConfigRegistry m_cvarsRegistry;
	std::string m_configFile;
};