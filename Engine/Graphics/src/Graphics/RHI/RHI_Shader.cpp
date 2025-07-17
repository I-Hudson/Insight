#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/RenderContext.h"

#if defined(IS_VULKAN_ENABLED)
#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#endif
#if defined(IS_DX12_ENABLED)
#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#endif

#include "Graphics/Vertex.h"

#include "Core/Memory.h"
#include "Core/Logger.h"
#include "Core/EnginePaths.h"
#include "Platforms/Platform.h"

#include "FileSystem/FileSystem.h"

#include "dxcapi.h"
#include "spirv_reflect.h"

#include <string>
#include <algorithm>

namespace Insight
{
	namespace Graphics
	{
		constexpr const char* SHADER_DYNAMIC_TAG = "__dynamic__";

		RHI_Shader* RHI_Shader::New()
		{
#if defined(IS_VULKAN_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_Shader_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#if defined(IS_DX12_ENABLED)
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_Shader_DX12, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
			return nullptr;
		}


		RHI_ShaderManager::RHI_ShaderManager()
		{ }

		RHI_ShaderManager::~RHI_ShaderManager()
		{ }

		RHI_Shader* RHI_ShaderManager::GetOrCreateShader(ShaderDesc desc)
		{
			if (!desc.IsValid())
			{
				return nullptr;
			}

			const u64 hash = desc.GetHash();

			std::lock_guard shaderLock(m_shaderLock);
			auto itr = m_shaders.find(hash);
			if (itr != m_shaders.end())
			{
				return itr->second;
			}

			RHI_Shader* shader = RHI_Shader::New();
			shader->Create(m_context, desc);
			shader->m_desc = desc;
			m_shaders[hash] = shader;

			return shader;
		}

		void RHI_ShaderManager::ReloadShaders()
		{
			std::lock_guard lock(m_shadersToReloadLock);
			if (!m_shadersToReload.empty())
			{
				m_context->GpuWaitForIdle();
				for (size_t i = 0; i < m_shadersToReload.size(); ++i)
				{
					RHI_Shader* shader = m_shadersToReload[i];
					shader->Destroy();
					shader->Create(m_context, shader->GetDesc());

					m_context->GetPipelineManager().DestroyPipelineWithShader(shader->GetDesc());
				}

				m_shadersToReload.clear();
			}
		}

		void RHI_ShaderManager::ReloadShader(std::string_view shaderName)
		{
			RHI_Shader* shaderPtr = nullptr;
			std::lock_guard shaderLock(m_shaderLock);
			for (const auto& [hash, shader] : m_shaders)
			{
				if (shader->GetDesc().ShaderName == shaderName) 
				{
					shaderPtr = shader;
					break;
				}
			}

			ReloadShader(shaderPtr);
		}

		void RHI_ShaderManager::ReloadShader(RHI_Shader* shader)
		{
			if (!shader)
			{
				IS_LOG_CORE_WARN("[RHI_ShaderManager::ReloadShader] Null shader pointer.");
				return;
			}
			std::lock_guard lock(m_shadersToReloadLock);
			m_shadersToReload.push_back(shader);
		}

		void RHI_ShaderManager::DestroyShader(RHI_Shader* shader)
		{
			if (!shader)
			{
				return;
			}

			const ShaderDesc desc = shader->GetDesc();
			if (!desc.IsValid())
			{
				return;
			}

			const u64 hash = desc.GetHash();

			std::lock_guard shaderLock(m_shaderLock);
			auto itr = m_shaders.find(hash);
			if (itr != m_shaders.end())
			{
				RenderContext::Instance().GpuWaitForIdle();

				m_context->GetPipelineManager().DestroyPipelineWithShader(desc);

				shader->Release();
				m_shaders.erase(hash);
			}
		}

		std::vector<RHI_Shader*> RHI_ShaderManager::GetAllShaders() const
		{
			std::vector<RHI_Shader*> shaders;
			std::lock_guard shaderLock(m_shaderLock);
			for (auto& [desc, shader] : m_shaders)
			{
				shaders.push_back(shader);
			}
			return shaders;
		}

		void RHI_ShaderManager::Destroy()
		{
			std::lock_guard shaderLock(m_shaderLock);
			for (auto& pair : m_shaders)
			{
				pair.second->Destroy();
				DeleteTracked(pair.second);
			}
			m_shaders.clear();
		}


		//// <summary>
		//// ShaderCompiler
		//// </summary>
		ShaderCompiler::ShaderCompiler()
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

		ShaderCompiler::~ShaderCompiler()
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

		IDxcBlob* ShaderCompiler::Compile(ShaderStageFlagBits stage, std::string_view filePath, ShaderCompilerLanguage languageToCompileTo)
		{
			/// Load the HLSL text shader from disk
			uint32_t codePage = CP_UTF8;
			IDxcBlobEncoding* sourceBlob;
			std::wstring filePathW = Platform::WStringFromStringView(filePath);
			ASSERT(SUCCEEDED(DXUtils->LoadFile(filePathW.c_str(), nullptr, &sourceBlob)));

			std::vector<Byte> shaderData;
			shaderData.resize(sourceBlob->GetBufferSize());
			Platform::MemCopy(shaderData.data(), sourceBlob->GetBufferPointer(), shaderData.size());
			sourceBlob->Release();

			IDxcBlob* compiledShaderBlob = Compile(stage, std::string(filePath), shaderData, languageToCompileTo);

			return compiledShaderBlob;
		}

		IDxcBlob* ShaderCompiler::Compile(ShaderStageFlagBits stage, std::string name, const std::vector<Byte>& shaderData, ShaderCompilerLanguage languageToCompileTo)
		{
			ASSERT(shaderData.size() > 0);
			m_languageToCompileTo = languageToCompileTo;

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
			ASSERT(SUCCEEDED(DXUtils->CreateDefaultIncludeHandler(&pIncludeHandler)));

			DxcBuffer Source;
			Source.Ptr = shaderData.data();
			Source.Size = shaderData.size();
			Source.Encoding = DXC_CP_UTF8; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

			// Set up arguments to be passed to the shader compiler

			std::vector<LPCWCHAR> arguments;

			std::wstring mainFunc = Platform::WStringFromString(StageToFuncName(stage));
			std::wstring targetProfile = Platform::WStringFromString(StageToProfileTarget(stage));

			// Entry point
			arguments.push_back(L"-E");
			arguments.push_back(mainFunc.c_str());

			/// Select target profile based on shader file extension
			arguments.push_back(L"-T");
			arguments.push_back(targetProfile.c_str());

			std::string resourcePath = EnginePaths::GetResourcePath() + "/Shaders/hlsl";
			std::wstring wResourcePath = Platform::WStringFromString(resourcePath);
			const wchar_t* c_Include_Directory = L"-I";
			arguments.push_back(c_Include_Directory);
			arguments.push_back(wResourcePath.c_str());

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

			arguments.push_back(L"-Wno-null-character");


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
			ASSERT(SUCCEEDED(DXCompiler->Compile(
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
			ASSERT(SUCCEEDED(DXCompiler->Compile(
				&Source,								// Source buffer.
				arguments.data(),						// Array of pointers to arguments.
				static_cast<UINT>(arguments.size()),	// Number of arguments.
				pIncludeHandler,						// User-provided interface to handle #include directives (optional).
				IID_PPV_ARGS(&ShaderReflectionResults)	// Compiler output status, buffer, and errors.
			)));

			// Print errors if present.
			IDxcBlobUtf8* pErrors = nullptr;
			ASSERT(SUCCEEDED(ShaderCompileResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)));
			// Note that d3dcompiler would return null if no errors or warnings are present.  
			// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
			if (pErrors != nullptr && pErrors->GetStringLength() != 0)
			{
				IS_LOG_CORE_ERROR(fmt::format("Shader compilation failed '{}': \n\n{}", name.c_str(), pErrors->GetStringPointer()));
			}
			pErrors->Release();

			pErrors = nullptr;
			ASSERT(SUCCEEDED(ShaderReflectionResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)));
			// Note that d3dcompiler would return null if no errors or warnings are present.  
			// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
			if (pErrors != nullptr && pErrors->GetStringLength() != 0)
			{
				LPCSTR strPointer = pErrors->GetStringPointer();
				SIZE_T strLength = pErrors->GetBufferSize();
				IS_LOG_CORE_ERROR(fmt::format("Shader compilation failed : \n\nName: {} \n\n Error: {}", name.c_str(), strPointer));
			}
			pErrors->Release();

			// Write shader to disk.
			std::ofstream shaderDisk;

			int startShaderFile = (int)name.find_last_of('/') + 1;
			int offsetShaderFile = (int)name.find_last_of('.') - startShaderFile;

			std::string_view shaderToDiskView = name.substr(startShaderFile, offsetShaderFile);

			std::string shaderCSOFolderPath = EnginePaths::GetExecutablePath() + "/Shader/CSO/";
			std::string shaderPDBFolderPath = EnginePaths::GetExecutablePath() + "/Shader/PDB/";
			std::string shaderReflectFolderPath = EnginePaths::GetExecutablePath() + "/Shader/Reflect/";
			FileSystem::CreateFolder(shaderCSOFolderPath);
			FileSystem::CreateFolder(shaderPDBFolderPath);
			FileSystem::CreateFolder(shaderReflectFolderPath);

			if (argDebugData)
			{
				IDxcBlob* pDebugData = nullptr;
				IDxcBlobUtf16* pDebugDataPath = nullptr;
				ShaderCompileResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);
				if (pDebugData && pDebugDataPath)
				{
					std::string debugPath = shaderPDBFolderPath + Platform::StringFromWString(pDebugDataPath->GetStringPointer());

					shaderDisk.open(debugPath.c_str(), std::ios::trunc);
					if (shaderDisk.is_open())
					{
						shaderDisk.write((const char*)pDebugData->GetBufferPointer(), pDebugData->GetBufferSize());
						shaderDisk.close();
					}

					debugPath = shaderPDBFolderPath + name + "_" + StageToFuncName(stage) + "_" + StageToProfileTarget(stage) + ".pdb";
					shaderDisk.open(debugPath.c_str(), std::ios::trunc);
					if (shaderDisk.is_open())
					{
						shaderDisk.write((const char*)pDebugData->GetBufferPointer(), pDebugData->GetBufferSize());
						shaderDisk.close();
					}

					pDebugData->Release();
					pDebugDataPath->Release();
				}
				//ComPtr<IDxcBlob> pReflectData;
				//ShaderReflectionResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(pReflectData.GetAddressOf()), nullptr);

				//const std::string reflectPath = shaderReflectFolderPath + name + "_" + StageToFuncName(stage) + "_" + StageToProfileTarget(stage);
				//shaderDisk.open(reflectPath.c_str(), std::ios::trunc);
				//if (shaderDisk.is_open())
				//{
				//	shaderDisk.write((const char*)pReflectData->GetBufferPointer(), pReflectData->GetBufferSize());
				//	shaderDisk.close();
				//}
			}

			// Get compilation result
			IDxcBlob* shaderCompiledCode;
			ShaderCompileResults->GetResult(&shaderCompiledCode);

			std::string shaderToDisk = shaderCSOFolderPath + name + "_" + StageToFuncName(stage) + "_" + StageToProfileTarget(stage) + ".cso";
			shaderDisk.open(shaderToDisk.c_str(), std::ios::trunc);
			if (shaderDisk.is_open())
			{
				shaderDisk.write((const char*)shaderCompiledCode->GetBufferPointer(), shaderCompiledCode->GetBufferSize());
				shaderDisk.close();
			}

			pIncludeHandler->Release();
			return shaderCompiledCode;
		}

		void ShaderCompiler::GetDescriptorSets(ShaderStageFlagBits stage, std::vector<DescriptorSet>& descriptor_sets, PushConstant& push_constant)
		{
			if (!ShaderReflectionResults)
			{
				IS_LOG_CORE_ERROR("[ShaderCompiler::GetDescriptorSets] Trying to extract descriptors but no shader has been compiled.");
				return;
			}

			IDxcBlob* code;
			ShaderReflectionResults->GetResult(&code);

			// Generate reflection data for a shader
			SpvReflectShaderModule module;
			SpvReflectResult result = spvReflectCreateShaderModule(code->GetBufferSize(), code->GetBufferPointer(), &module);
			ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			code->Release();

			// Output variables, descriptor bindings, descriptor sets, and push constants
			// can be enumerated and extracted using a similar mechanism.

			u32 descriptorSetCount = 0;
			result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, NULL);
			ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectDescriptorSet*> descriptorSets;
			descriptorSets.resize(descriptorSetCount);
			result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, descriptorSets.data());
			ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			auto findPreviousDescriptor = [](const DescriptorBinding& descriptoToFind, const DescriptorSet& descriptor_set)
				-> std::pair<bool, DescriptorBinding*>
			{
				for (auto& desc : descriptor_set.Bindings)
				{
					if (descriptoToFind.Set == desc.Set
						&& descriptoToFind.Binding == desc.Binding
						&& descriptoToFind.Size == desc.Size
						&& descriptoToFind.Type == desc.Type)
					{
						return std::make_pair(true, &const_cast<DescriptorBinding&>(desc));
					}
				}
				return std::make_pair(false, nullptr);
			};

			auto get_descriptor_set = [](int set, std::vector<DescriptorSet>& descriptor_sets) -> DescriptorSet*
			{
				for (size_t i = 0; i < descriptor_sets.size(); ++i)
				{
					if (descriptor_sets.at(i).Set == set) 
					{
						return &descriptor_sets.at(i);
					}
				}
				return nullptr;
			};

			for (size_t i = 0; i < descriptorSets.size(); ++i)
			{
				const SpvReflectDescriptorSet& descriptorSet = *descriptorSets[i];
				DescriptorSet* descriptor_set = get_descriptor_set(descriptorSet.set, descriptor_sets);
				if (descriptor_set == nullptr)
				{
					descriptor_sets.push_back(DescriptorSet(std::to_string(descriptorSet.set), descriptorSet.set, {}));
					descriptor_set = &descriptor_sets.back();
				}

				descriptor_set->Stages |= stage;

				for (size_t j = 0; j < descriptorSet.binding_count; ++j)
				{
					const SpvReflectDescriptorBinding& binding = *descriptorSet.bindings[j];
					const SpvReflectBlockVariable& block = descriptorSet.bindings[j]->block;

					DescriptorType descriptor_type = SpvReflectDescriptorTypeToDescriptorType(binding.descriptor_type);
					if (std::string(binding.name).find(SHADER_DYNAMIC_TAG) != std::string::npos)
					{
						descriptor_type = DescriptorType::Uniform_Buffer_Dynamic;
					}

					DescriptorBinding descriptor(
						binding.set,
						binding.binding,
						stage,
						block.size,
						binding.count,
						descriptor_type);
					descriptor.SetHashs();

					auto [foundDescriptor, descriptorFound] = findPreviousDescriptor(descriptor, *descriptor_set);
					if (foundDescriptor)
					{
						descriptorFound->Stages |= stage;
					}
					else
					{
						descriptor_set->Bindings.push_back(descriptor);
					}
				}
				descriptor_set->SetHashs();
			}

			// Get the push constants.
			u32 push_constant_blocks_count = 0;
			result = spvReflectEnumeratePushConstantBlocks(&module, &push_constant_blocks_count, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			ASSERT(push_constant_blocks_count <= 1);

			std::vector<SpvReflectBlockVariable*> push_constants;
			push_constants.resize(push_constant_blocks_count);
			result = spvReflectEnumeratePushConstantBlocks(&module, &push_constant_blocks_count, push_constants.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			for (size_t i = 0; i < push_constants.size(); ++i)
			{
				SpvReflectBlockVariable* block = push_constants.at(i);
				push_constant.Size = block->size;
				push_constant.ShaderStages |= stage;
			}

			/// Destroy the reflection data when no longer required.
			spvReflectDestroyShaderModule(&module);

			// Order all the bindings within the sets.
			for (size_t i = 0; i < descriptor_sets.size(); ++i)
			{
				std::sort(descriptor_sets.at(i).Bindings.begin(), descriptor_sets.at(i).Bindings.end(), [](const DescriptorBinding& b1, const DescriptorBinding& b2)
					{
						return b1.Binding < b2.Binding;
					});
				std::sort(descriptor_sets.at(i).Bindings.begin(), descriptor_sets.at(i).Bindings.end(), [](const DescriptorBinding& b1, const DescriptorBinding& b2)
					{
						return b1.Binding != b2.Binding ? 0 : 
						(b1.Type == DescriptorType::Unifom_Buffer 
						|| b1.Type == DescriptorType::Storage_Buffer) 
						&& 
						(b2.Type != DescriptorType::Unifom_Buffer
						|| b2.Type != DescriptorType::Storage_Buffer)
						? 0 : 0;
					});
			}

			// Order all the sets.
			std::sort(descriptor_sets.begin(), descriptor_sets.end(), [](const DescriptorSet& set1, const DescriptorSet& set2)
				{
					return set1.Set < set2.Set;
				});

			bool filledInSets = false;
			while (!filledInSets)
			{
				filledInSets = true;
				for (size_t i = 0; i < descriptor_sets.size(); ++i)
				{
					if (descriptor_sets.at(i).Set != i)
					{
						filledInSets = false;
						descriptor_sets.insert(descriptor_sets.begin() + i, DescriptorSet("Bindless", static_cast<int>(i), {}));
						break;
					}
				}
			}
		}

		u32 SpvFormatToByteSize(SpvReflectFormat format)
		{
			switch (format)
			{
			case SPV_REFLECT_FORMAT_UNDEFINED: assert(false);
			case SPV_REFLECT_FORMAT_R32_UINT:				return 4;
			case SPV_REFLECT_FORMAT_R32_SINT:				return 4;
			case SPV_REFLECT_FORMAT_R32_SFLOAT:				return 4;
			case SPV_REFLECT_FORMAT_R32G32_UINT:			return 8;
			case SPV_REFLECT_FORMAT_R32G32_SINT:			return 8;
			case SPV_REFLECT_FORMAT_R32G32_SFLOAT:			return 8;
			case SPV_REFLECT_FORMAT_R32G32B32_UINT:			return 12;
			case SPV_REFLECT_FORMAT_R32G32B32_SINT:			return 12;
			case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:		return 12;
			case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:		return 16;
			case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:		return 16;
			case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:	return 16;
			case SPV_REFLECT_FORMAT_R64_UINT:				return 8;
			case SPV_REFLECT_FORMAT_R64_SINT:				return 8;
			case SPV_REFLECT_FORMAT_R64_SFLOAT:				return 8;
			case SPV_REFLECT_FORMAT_R64G64_UINT:			return 16;
			case SPV_REFLECT_FORMAT_R64G64_SINT:			return 16;
			case SPV_REFLECT_FORMAT_R64G64_SFLOAT:			return 16;
			case SPV_REFLECT_FORMAT_R64G64B64_UINT:			return 24;
			case SPV_REFLECT_FORMAT_R64G64B64_SINT:			return 24;
			case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:		return 24;
			case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:		return 32;
			case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:		return 32;
			case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:	return 32;
			default:  assert(false);
			}
			return 0;
		}

		PixelFormat SpvFormatToPixelFormat(SpvReflectFormat frt)
		{
			switch (frt)
			{
			case SPV_REFLECT_FORMAT_UNDEFINED:				return PixelFormat::Unknown;
			case SPV_REFLECT_FORMAT_R32_UINT:				return PixelFormat::R32_UInt;
			case SPV_REFLECT_FORMAT_R32_SINT:				return PixelFormat::R32_SInt;
			case SPV_REFLECT_FORMAT_R32_SFLOAT:				return PixelFormat::R32_Float;
			case SPV_REFLECT_FORMAT_R32G32_UINT:			return PixelFormat::R32G32_UInt;
			case SPV_REFLECT_FORMAT_R32G32_SINT:			return PixelFormat::R32G32_SInt;
			case SPV_REFLECT_FORMAT_R32G32_SFLOAT:			return PixelFormat::R32G32_Float;
			case SPV_REFLECT_FORMAT_R32G32B32_UINT:			return PixelFormat::R32G32B32_UInt;
			case SPV_REFLECT_FORMAT_R32G32B32_SINT:			return PixelFormat::R32G32B32_SInt;
			case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:		return PixelFormat::R32G32B32_Float;
			case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:		return PixelFormat::R32G32B32A32_UInt;
			case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:		return PixelFormat::R32G32B32A32_SInt;
			case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:	return PixelFormat::R32G32B32A32_Float;
			case SPV_REFLECT_FORMAT_R64_UINT:				assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64_SINT:				assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64_SFLOAT:				assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64_UINT:			assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64_SINT:			assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64_SFLOAT:			assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64B64_UINT:			assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64B64_SINT:			assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:		assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:		assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:		assert(false && "Format not supported.");
			case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:	assert(false && "Format not supported.");
			}
			return PixelFormat::Unknown;
		}

		std::vector<ShaderInputLayout> ShaderCompiler::GetInputLayout()
		{
			if (!ShaderReflectionResults)
			{
				IS_LOG_CORE_ERROR("[ShaderCompiler::GetInputLayout] Trying to get the input layout but no shader has been compiled.");
				return std::vector<ShaderInputLayout>();
			}

			IDxcBlob* code;
			ShaderReflectionResults->GetResult(&code);

			/// Generate reflection data for a shader
			SpvReflectShaderModule module;
			SpvReflectResult result = spvReflectCreateShaderModule(code->GetBufferSize(), code->GetBufferPointer(), &module);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			code->Release();

			/// Enumerate and extract shader's input variables
			uint32_t inputCount = 0;
			result = spvReflectEnumerateInputVariables(&module, &inputCount, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectInterfaceVariable*> inputVars;
			inputVars.resize(inputCount);

			///SpvReflectInterfaceVariable** input_vars = inputVars (SpvReflectInterfaceVariable**)malloc(inputCount * sizeof(SpvReflectInterfaceVariable*));
			result = spvReflectEnumerateInputVariables(&module, &inputCount, inputVars.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<ShaderInputLayout> inputLayout;
			int stride = 0;

			for (size_t i = 0; i < inputCount; i++)
			{
				SpvReflectInterfaceVariable* interfaceVariable = inputVars.at(i);
				if (interfaceVariable->built_in != -1)
				{
					continue;

				}
				std::string name = interfaceVariable->name;
				name = name.substr(name.find_last_of('.') + 1);

				ShaderInputLayout layout(
					interfaceVariable->location,
					SpvFormatToPixelFormat(interfaceVariable->format),
#ifdef VERTEX_SPLIT_STREAMS
					i,
#else
					0,
#endif
					stride,
					std::move(name));
				inputLayout.push_back(layout);

				stride += SpvFormatToByteSize(interfaceVariable->format);
			}

			/// Destroy the reflection data when no longer required.
			spvReflectDestroyShaderModule(&module);

			return inputLayout;
		}


		std::string ShaderCompiler::StageToFuncName(ShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case ShaderStageFlagBits::ShaderStage_Vertex: return "VSMain";
			case ShaderStageFlagBits::ShaderStage_TessControl: return "TSMain";
			case ShaderStageFlagBits::ShaderStage_TessEval: return "TEMain";
			case ShaderStageFlagBits::ShaderStage_Geometry: return "GSMain";
			case ShaderStageFlagBits::ShaderStage_Pixel: return "PSMain";
			case ShaderStageFlagBits::ShaderStage_Compute: return "CSMain";
				break;
			}
			return "";
		}

		std::string ShaderCompiler::StageToProfileTarget(ShaderStageFlagBits stage)
		{
			switch (stage)
			{
			case ShaderStageFlagBits::ShaderStage_Vertex: return "vs_6_1";
			case ShaderStageFlagBits::ShaderStage_TessControl: return "hs_6_1";
			case ShaderStageFlagBits::ShaderStage_TessEval: return "te_6_1";
			case ShaderStageFlagBits::ShaderStage_Geometry: return "gs_6_1";
			case ShaderStageFlagBits::ShaderStage_Pixel: return "ps_6_1";
			case ShaderStageFlagBits::ShaderStage_Compute: return "cs_6_1";
				break;
			}
			FAIL_ASSERT();
			return "";
		}

		DescriptorType ShaderCompiler::SpvReflectDescriptorTypeToDescriptorType(u32 type)
		{
			switch (type)
			{
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:						return DescriptorType::Sampler;
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:					return DescriptorType::Sampled_Image;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:					return DescriptorType::Storage_Image;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:			return DescriptorType::Uniform_Texel_Buffer;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:			return DescriptorType::Storage_Texel_Buffer;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:				return DescriptorType::Unifom_Buffer;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:				return DescriptorType::Storage_Buffer;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:		return DescriptorType::Uniform_Buffer_Dynamic;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:		return DescriptorType::Storage_Buffer_Dyanmic;
			case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:				return DescriptorType::Input_Attachment;
			case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:	return DescriptorType::Unknown;
			default:
				break;
			}
			FAIL_ASSERT();
			return DescriptorType::Unknown;
		}

		DescriptorResourceType ShaderCompiler::SpvReflectDescriptorResourceTypeToDescriptorResourceType(u32 type)
		{
			switch (type)
			{
			case SPV_REFLECT_RESOURCE_FLAG_UNDEFINED:	return DescriptorResourceType::Unknown;
			case SPV_REFLECT_RESOURCE_FLAG_SAMPLER:		return DescriptorResourceType::Sampler;
			case SPV_REFLECT_RESOURCE_FLAG_CBV:			return DescriptorResourceType::CBV;
			case SPV_REFLECT_RESOURCE_FLAG_SRV:			return DescriptorResourceType::SRV;
			case SPV_REFLECT_RESOURCE_FLAG_UAV:			return DescriptorResourceType::UAV;
			default:
				break;
			}
			FAIL_ASSERT();
			return DescriptorResourceType::Unknown;
		}
	}
}