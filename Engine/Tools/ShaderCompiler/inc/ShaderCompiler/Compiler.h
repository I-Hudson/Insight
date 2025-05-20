#pragma once

#include <string>
#include <vector>

struct IDxcBlob;
struct IDxcUtils;
struct IDxcCompiler3;
struct IDxcResult;

namespace Insight::ShaderCompiler
{
	enum class ShaderCompilerLanguage
	{
		Spirv,
		Hlsl
	};

	enum class ShderStageFlag
	{
		Vertex,
		TessControl,
		TessEval,
		Geometry,
		Pixel,
		Compute,
	};

	struct CompileOptions
	{
		std::vector<std::string> IncludeDirs;
		std::string CSOPath;
		std::string PDBPath;
	};

	typedef void(*CompilerErrorCallback)(const char*);

	// TODO Low: Replace IDxc* with a ref counter object. (This needs to be custom due to compile time)
    class Compiler
    {
	public:
		Compiler();
		Compiler(const Compiler& other) = delete;
		Compiler(Compiler&& other) = delete;
		~Compiler();

		IDxcBlob* Compile(const ShderStageFlag stage, std::string_view filePath, ShaderCompilerLanguage languageToCompileTo, const CompileOptions& options);
		IDxcBlob* Compile(const ShderStageFlag stage, std::string name, const std::vector<unsigned char>& shaderData, ShaderCompilerLanguage languageToCompileTo, const CompileOptions& options);

		void SetErrorCallback(CompilerErrorCallback callback) { m_erroCallback = callback; }

	private:
		std::wstring StageToFuncName(const ShderStageFlag stage);
		std::wstring StageToProfileTarget(const ShderStageFlag stage);

		CompilerErrorCallback m_erroCallback = nullptr;

		IDxcUtils* DXUtils = nullptr;
		IDxcCompiler3* DXCompiler = nullptr;

		IDxcResult* ShaderCompileResults = nullptr;
		IDxcResult* ShaderReflectionResults = nullptr;

		ShaderCompilerLanguage m_languageToCompileTo;
	};
}