#include "ShaderCompiler/Compiler.h"

#include <filesystem>
#include <fstream>

#ifdef IS_PLATFORM_WINDOWS
//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//#elif defined()
#endif

#include "dxcapi.h"

#include <algorithm>
#include <cassert>

namespace Insight::ShaderCompiler
{
	constexpr const char* SHADER_DYNAMIC_TAG = "__dynamic__";

	std::string FormatError(const char* format, ...)
	{
		constexpr uint64_t messageBufferSize = 2048;

		char buffer[messageBufferSize];
		va_list args;
		va_start(args, format);
		uint64_t bufferIdx = vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		return std::string(std::begin(buffer), std::begin(buffer) + bufferIdx);
	}

	Compiler::Compiler()
	{
		HRESULT hres;
		/// Initialize DXC library
		hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&DXUtils));
		if (FAILED(hres))
		{
			throw std::runtime_error("Could not init DXC Library");
		}

		/// Initialize the DXC compiler
		hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DXCompiler));
		if (FAILED(hres))
		{
			throw std::runtime_error("Could not init DXC Compiler");
		}
	}

	Compiler::~Compiler()
	{
		if (DXUtils)
		{
			DXUtils->Release();
		}
		if (DXCompiler)
		{
			DXCompiler->Release();
		}
		if (ShaderCompileResults)
		{
			ShaderCompileResults->Release();
		}
		if (ShaderReflectionResults)
		{
			ShaderReflectionResults->Release();
		}
	}

	IDxcBlob* Compiler::Compile(const ShderStageFlag stage, std::string_view filePath, ShaderCompilerLanguage languageToCompileTo, const CompileOptions& options)
	{
		/// Load the HLSL text shader from disk
		uint32_t codePage = CP_UTF8;
		IDxcBlobEncoding* sourceBlob;
		std::wstring filePathW = std::wstring(filePath.begin(), filePath.end());
		assert(SUCCEEDED(DXUtils->LoadFile(filePathW.c_str(), nullptr, &sourceBlob)));

		std::vector<byte> shaderData;
		shaderData.resize(sourceBlob->GetBufferSize());
		memcpy(shaderData.data(), sourceBlob->GetBufferPointer(), shaderData.size());
		sourceBlob->Release();

		IDxcBlob* compiledShaderBlob = Compile(stage, std::string(filePath), shaderData, languageToCompileTo, options);

		return compiledShaderBlob;
	}

	IDxcBlob* Compiler::Compile(const ShderStageFlag stage, std::string name, const std::vector<unsigned char>& shaderData, ShaderCompilerLanguage languageToCompileTo, const CompileOptions& options)
	{
		assert(shaderData.size() > 0);
		m_languageToCompileTo = languageToCompileTo;
		const std::wstring wName = std::wstring(name.begin(), name.end());

		if (ShaderCompileResults)
		{
			ShaderCompileResults->Release();
		}
		if (ShaderReflectionResults)
		{
			ShaderReflectionResults->Release();
		}

		/// Create default include handler. (You can create your own...)
		IDxcIncludeHandler* pIncludeHandler;
		assert(SUCCEEDED(DXUtils->CreateDefaultIncludeHandler(&pIncludeHandler)));

		DxcBuffer Source;
		Source.Ptr = shaderData.data();
		Source.Size = shaderData.size();
		Source.Encoding = DXC_CP_UTF8; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

		// Set up arguments to be passed to the shader compiler

		std::vector<LPCWCHAR> arguments;

		std::wstring mainFunc = StageToFuncName(stage);
		std::wstring targetProfile = StageToProfileTarget(stage);

		// Entry point
		arguments.push_back(L"-E");
		arguments.push_back(mainFunc.c_str());

		/// Select target profile based on shader file extension
		arguments.push_back(L"-T");
		arguments.push_back(targetProfile.c_str());

		for (size_t i = 0; i < options.IncludeDirs.size(); ++i)
		{
			const std::string& includeDir = options.IncludeDirs[i];
			std::wstring wIncludeDir = std::wstring(includeDir.begin(), includeDir.end());
			const wchar_t* c_Include_Directory = L"-I";
			arguments.push_back(c_Include_Directory);
			arguments.push_back(wIncludeDir.c_str());
		}

		const bool argDebugData = true;
		const bool argOptimisationsEnabled = false;
		if (argDebugData)
		{
			arguments.push_back(DXC_ARG_DEBUG);
		}
		if (argOptimisationsEnabled)
		{
			arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
		}
		//arguments.push_back(DXC_ARG_DEBUG_NAME_FOR_BINARY);

		arguments.push_back(L"-Qstrip_debug");
		//arguments.push_back(L"-Qstrip_reflect");

		arguments.push_back(L"-Wnull-character");


		// Tell the compiler to output SPIR-V
		if (languageToCompileTo == ShaderCompilerLanguage::Spirv)
		{
			arguments.push_back(L"-D");
			arguments.push_back(L"VULKAN");
			arguments.push_back(L"-spirv");

			//arguments.push_back(L"-fvk-auto-shift-bindings");
			//arguments.push_back(L"-fspv-target-env=vulkan1.2");
			//arguments.push_back(L"-fvk-b-shift"); arguments.push_back(L"1000"); arguments.push_back(L"0");
			arguments.push_back(L"-fvk-t-shift"); arguments.push_back(L"1000"); arguments.push_back(L"all");
			arguments.push_back(L"-fvk-u-shift"); arguments.push_back(L"2000"); arguments.push_back(L"all");
		}
		else
		{
			arguments.push_back(L"-D");
			arguments.push_back(L"DX12");
		}

#ifdef VERTEX_NORMAL_PACKED
		arguments.push_back(L"-D");
		arguments.push_back(L"VERTEX_NORMAL_PACKED");
#endif
#ifdef VERTEX_COLOUR_PACKED
		arguments.push_back(L"-D");
		arguments.push_back(L"VERTEX_COLOUR_PACKED");
#endif
#ifdef VERTEX_UV_PACKED
		arguments.push_back(L"-D");
		arguments.push_back(L"VERTEX_UV_PACKED");
#endif
#ifdef VERTEX_BONE_ID_PACKED
		arguments.push_back(L"-D");
		arguments.push_back(L"VERTEX_BONE_ID_PACKED");
#endif
#ifdef VERTEX_BONE_WEIGHT_PACKED
		arguments.push_back(L"-D");
		arguments.push_back(L"VERTEX_BONE_WEIGHT_PACKED");
#endif

		// Compile shader
		assert(SUCCEEDED(DXCompiler->Compile(
			&Source,									// Source buffer.
			arguments.data(),							// Array of pointers to arguments.
			static_cast<UINT>(arguments.size()),		// Number of arguments.
			pIncludeHandler,							// User-provided interface to handle #include directives (optional).
			IID_PPV_ARGS(&ShaderCompileResults)			// Compiler output status, buffer, and errors.
		)));

		if (languageToCompileTo != ShaderCompilerLanguage::Spirv)
		{
			arguments.push_back(L"-spirv");
		}

		arguments.push_back(L"-fspv-reflect");
		assert(SUCCEEDED(DXCompiler->Compile(
			&Source,								// Source buffer.
			arguments.data(),						// Array of pointers to arguments.
			static_cast<UINT>(arguments.size()),	// Number of arguments.
			pIncludeHandler,						// User-provided interface to handle #include directives (optional).
			IID_PPV_ARGS(&ShaderReflectionResults)	// Compiler output status, buffer, and errors.
		)));

		// Print errors if present.
		IDxcBlobUtf8* pErrors = nullptr;
		assert(SUCCEEDED(ShaderCompileResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)));
		// Note that d3dcompiler would return null if no errors or warnings are present.  
		// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		{
			if (m_erroCallback)
			{
				m_erroCallback(FormatError("Shader compilation failed : \n\n%s", pErrors->GetStringPointer()).c_str());
			}
		}
		pErrors->Release();

		pErrors = nullptr;
		assert(SUCCEEDED(ShaderReflectionResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)));
		// Note that d3dcompiler would return null if no errors or warnings are present.  
		// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		{
			LPCSTR strPointer = pErrors->GetStringPointer();
			SIZE_T strLength = pErrors->GetBufferSize();
			if (m_erroCallback)
			{
				m_erroCallback(FormatError("Shader compilation failed : \n\nName: %s \n\n Error: %s", name.c_str(), strPointer).c_str());
			}
		}
		pErrors->Release();

		// Write shader to disk.
		std::ofstream shaderDisk;

		int startShaderFile = (int)name.find_last_of('/') + 1;
		int offsetShaderFile = (int)name.find_last_of('.') - startShaderFile;

		std::string_view shaderToDiskView = name.substr(startShaderFile, offsetShaderFile);

		const std::wstring& shaderCSOFolderPath = std::wstring(options.CSOPath.begin(), options.CSOPath.end());
		if (!std::filesystem::exists(shaderCSOFolderPath))
		{
			std::filesystem::create_directories(shaderCSOFolderPath);
		}

		const std::wstring& shaderPDBFolderPath = std::wstring(options.PDBPath.begin(), options.PDBPath.end());
		if (!std::filesystem::exists(shaderPDBFolderPath))
		{
			std::filesystem::create_directories(shaderPDBFolderPath);
		}

		if (argDebugData)
		{
			IDxcBlob* pDebugData = nullptr;
			IDxcBlobUtf16* pDebugDataPath = nullptr;
			ShaderCompileResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);
			if (pDebugData && pDebugDataPath)
			{
				std::wstring debugPath = shaderPDBFolderPath + pDebugDataPath->GetStringPointer();

				shaderDisk.open(debugPath.c_str(), std::ios::trunc);
				if (shaderDisk.is_open())
				{
					shaderDisk.write((const char*)pDebugData->GetBufferPointer(), pDebugData->GetBufferSize());
					shaderDisk.close();
				}

				debugPath = shaderPDBFolderPath + wName + L"_" + StageToFuncName(stage) + L"_" + StageToProfileTarget(stage) + L".pdb";
				shaderDisk.open(debugPath.c_str(), std::ios::trunc);
				if (shaderDisk.is_open())
				{
					shaderDisk.write((const char*)pDebugData->GetBufferPointer(), pDebugData->GetBufferSize());
					shaderDisk.close();
				}

				pDebugData->Release();
				pDebugDataPath->Release();
			}
		}

		// Get compilation result
		IDxcBlob* shaderCompiledCode;
		ShaderCompileResults->GetResult(&shaderCompiledCode);

		std::wstring shaderToDisk = shaderCSOFolderPath + wName + L"_" + StageToFuncName(stage) + L"_" + StageToProfileTarget(stage) + L".cso";
		shaderDisk.open(shaderToDisk.c_str(), std::ios::trunc);
		if (shaderDisk.is_open())
		{
			shaderDisk.write((const char*)shaderCompiledCode->GetBufferPointer(), shaderCompiledCode->GetBufferSize());
			shaderDisk.close();
		}

		pIncludeHandler->Release();
		return shaderCompiledCode;
	}

	std::wstring Compiler::StageToFuncName(const ShderStageFlag stage)
	{
		switch (stage)
		{
		case ShderStageFlag::Vertex: return L"VSMain";
		case ShderStageFlag::TessControl: return L"TSMain";
		case ShderStageFlag::TessEval: return L"TEMain";
		case ShderStageFlag::Geometry: return L"GSMain";
		case ShderStageFlag::Pixel: return L"PSMain";
		case ShderStageFlag::Compute: return L"CSMain";
			break;
		}
		return L"";
	}

	std::wstring Compiler::StageToProfileTarget(const ShderStageFlag stage)
	{
		switch (stage)
		{
		case ShderStageFlag::Vertex: return L"vs_6_1";
		case ShderStageFlag::TessControl: return L"hs_6_1";
		case ShderStageFlag::TessEval: return L"te_6_1";
		case ShderStageFlag::Geometry: return L"gs_6_1";
		case ShderStageFlag::Pixel: return L"ps_6_1";
		case ShderStageFlag::Compute: return L"cs_6_1";
			break;
		}
		assert(false);
		return L"";
	}
}