#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Core/Logger.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <fstream>

// PDB not found for glslang
#pragma warning( disable : 4099 )

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			DX12::ComPtr<IDxcUtils> RHI_Shader_Vulkan::s_dxUtils;
			DX12::ComPtr<IDxcCompiler3> RHI_Shader_Vulkan::s_dxCompiler;
			int RHI_Shader_Vulkan::s_dxInUse;

			void RHI_Shader_Vulkan::Create(RenderContext* context, ShaderDesc desc)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);

				if (!desc.VertexFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eVertex, desc.VertexFilePath, 0); }
				if (!desc.TesselationControlFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eTessellationControl, desc.TesselationControlFilePath, 1); }
				if (!desc.TesselationEvaluationVertexFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eTessellationEvaluation, desc.TesselationEvaluationVertexFilePath, 2); }
				if (!desc.GeoemtyFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eGeometry, desc.GeoemtyFilePath, 3); }
				if (!desc.PixelFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eFragment, desc.PixelFilePath, 4); }

				CreateVertexInputLayout(desc);
			}

			void RHI_Shader_Vulkan::Destroy()
			{
				for (vk::ShaderModule& mod : m_modules)
				{
					if (mod)
					{
						m_context->GetDevice().destroyShaderModule(mod);
						mod = vk::ShaderModule(nullptr);
					}
				}

				--s_dxInUse;
				if (s_dxInUse == 0)
				{
					s_dxCompiler.Reset();
					s_dxUtils.Reset();
				}
			}

			void RHI_Shader_Vulkan::CompileStage(vk::ShaderStageFlagBits stageType, std::wstring_view path, int moduleIndex)
			{
				HRESULT hres;
				if (!s_dxUtils)
				{
					// Initialize DXC library
					hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_dxUtils));
					if (FAILED(hres)) {
						throw std::runtime_error("Could not init DXC Library");
					}

					// Initialize the DXC compiler
					hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_dxCompiler));
					if (FAILED(hres)) {
						throw std::runtime_error("Could not init DXC Compiler");
					}
				}
				++s_dxInUse;

				// Create default include handler. (You can create your own...)
				DX12::ComPtr<IDxcIncludeHandler> pIncludeHandler;
				s_dxUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

				// Load the HLSL text shader from disk
				uint32_t codePage = CP_UTF8;
				DX12::ComPtr<IDxcBlobEncoding> sourceBlob;
				hres = s_dxUtils->LoadFile(path.data(), nullptr, &sourceBlob);
				if (FAILED(hres))
				{
					throw std::runtime_error("Could not load shader file");
				}

				DxcBuffer Source;
				Source.Ptr = sourceBlob->GetBufferPointer();
				Source.Size = sourceBlob->GetBufferSize();
				Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.


				// Set up arguments to be passed to the shader compiler

				// Tell the compiler to output SPIR-V
				std::vector<LPCWSTR> arguments;
				arguments.push_back(L"-spirv");

				std::wstring mainFunc;
				std::wstring targetProfile;
				switch (stageType)
				{
				case vk::ShaderStageFlagBits::eVertex:
					mainFunc = L"VSMain";
					targetProfile = L"vs_6_1";
					break;
				case vk::ShaderStageFlagBits::eTessellationControl:
					mainFunc = L"TSMain";
					targetProfile = L"vs_6_1";
					break;
				case vk::ShaderStageFlagBits::eTessellationEvaluation:
					mainFunc = L"TEMain";
					targetProfile = L"vs_6_1";
					break;
				case vk::ShaderStageFlagBits::eGeometry:
					mainFunc = L"GSMain";
					targetProfile = L"vs_6_1";
					break;
				case vk::ShaderStageFlagBits::eFragment:
					mainFunc = L"PSMain";
					targetProfile = L"ps_6_1";
					break;
				case vk::ShaderStageFlagBits::eCompute:
					mainFunc = L"CSMain";
					targetProfile = L"vs_6_1";
					break;
				}
				// Entry point
				arguments.push_back(L"-E");
				arguments.push_back(mainFunc.c_str());

				// Select target profile based on shader file extension
				arguments.push_back(L"-T");
				arguments.push_back(targetProfile.c_str());

				// Compile shader
				DX12::ComPtr<IDxcResult> pResults;
				hres = s_dxCompiler->Compile(
					&Source,                // Source buffer.
					arguments.data(),       // Array of pointers to arguments.
					arguments.size(),		// Number of arguments.
					pIncludeHandler.Get(),	// User-provided interface to handle #include directives (optional).
					IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
				);
				// Print errors if present.
				//
				DX12::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
				pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
				// Note that d3dcompiler would return null if no errors or warnings are present.  
				// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
				if (pErrors != nullptr && pErrors->GetStringLength() != 0)
				{
					IS_CORE_ERROR(fmt::format("Shader compilation failed : \n\n{}", pErrors->GetStringPointer()));
				}

				// Get compilation result
				DX12::ComPtr<IDxcBlob> code;
				pResults->GetResult(&code);

				vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, {});
				createInfo.setCodeSize(code->GetBufferSize());
				createInfo.setPCode((u32*)code->GetBufferPointer());

				if (m_modules[moduleIndex])
				{
					m_context->GetDevice().destroyShaderModule(m_modules[moduleIndex]);
				}
				m_modules[moduleIndex] = m_context->GetDevice().createShaderModule(createInfo);
				m_mainFuncNames[moduleIndex] = mainFunc;
				if (!m_modules[moduleIndex])
				{
					IS_CORE_ERROR("Shader compilation failed.");
				}
			}

			void RHI_Shader_Vulkan::CreateVertexInputLayout(const ShaderDesc& desc)
			{
				m_vertexInputLayout = {};
				int stride = 0;
				for (const auto& input : desc.VertexLayout)
				{
					vk::VertexInputAttributeDescription attri = {};
					attri.location = input.Binding;
					attri.binding = 0;
					attri.format = PixelFormatToVulkan(input.Format);
					attri.offset = stride;
					stride += PixelFormatExtensions::SizeInBytes(input.Format);

					m_vertexInputLayout.Attributes.push_back(attri);
				}

				if (m_vertexInputLayout.Attributes.size() > 0)
				{
					vk::VertexInputBindingDescription inputDesc = {};
					inputDesc.binding = 0;
					inputDesc.stride = stride;
					inputDesc.inputRate = vk::VertexInputRate::eVertex;
					m_vertexInputLayout.Bindings.push_back(inputDesc);
					m_vertexInputLayout.CreateInfo = vk::PipelineVertexInputStateCreateInfo({}, m_vertexInputLayout.Bindings, m_vertexInputLayout.Attributes);
				}
			}
		}
	}
}

#pragma warning( default : 4099 )