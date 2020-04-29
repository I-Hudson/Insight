#include "ispch.h"
#include "ShaderModuleBase.h"
#include "Device.h"
#include "Insight/Renderer/VulkanInits.h"
#include "Insight/ShaderParser/ShaderParser.h"

#include "Insight/Log.h"

namespace Insight
{
	namespace Render
	{
		ShaderModuleBase::ShaderModuleBase(const Device* device, const std::string& filepath, const ShaderType& type)
			: m_device(device), m_type(type), m_shaderModule(VK_NULL_HANDLE)
		{
			m_shaderData = ShaderParser::ParseShader(filepath, type);

			auto spirvData = CompileGLSL(filepath);

			VkShaderModuleCreateInfo createInfo = VulkanInits::ShaderModuleInfo(spirvData);

			ThrowIfFailed(vkCreateShaderModule(device->GetDevice(), &createInfo, nullptr, &m_shaderModule));
		}

		ShaderModuleBase::~ShaderModuleBase()
		{ }

		void ShaderModuleBase::Destroy()
		{
			vkDestroyShaderModule(m_device->GetDevice(), m_shaderModule, nullptr);
		}

		VkShaderStageFlagBits ShaderModuleBase::GetShaderStageBit() const
		{
			switch (m_type)
			{
			case Insight::Render::None: break;
			case Insight::Render::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
			case Insight::Render::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
			case Insight::Render::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
			case Insight::Render::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
			}
		}

		ShaderType ShaderModuleBase::GetShaderType() const
		{
			return m_type;
		}

		VkPipelineVertexInputStateCreateInfo ShaderModuleBase::GetVertexInputCreateInfo()
		{
			auto desc = GetVertexBindingDesc();
			auto attributes = GetAttributes();

			VkPipelineVertexInputStateCreateInfo createInfo = VulkanInits::VertexInputInfo();
			createInfo.vertexBindingDescriptionCount = desc.stride != 0 ? 1 : 0;
			createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
			createInfo.pVertexBindingDescriptions = desc.stride != 0 ? &desc : nullptr;
			createInfo.pVertexAttributeDescriptions = attributes.data();

			return createInfo;
		}

		std::vector<VkDescriptorSetLayoutBinding> ShaderModuleBase::GetPipelineLayoutCreateInfo()
		{
			std::vector<VkDescriptorSetLayoutBinding> createInfos{};
			for (auto it = m_shaderData.UniformBlocks.begin();  it != m_shaderData.UniformBlocks.end(); ++it)
			{
				auto desc = VulkanInits::DescriptorSetLayoutBinding((*it).Binding, m_shaderData.UniformBlocks.size());
				desc.stageFlags = GetShaderStageBit();
				createInfos.push_back(desc);
			}

			return createInfos;
		}

		VkVertexInputBindingDescription ShaderModuleBase::GetVertexBindingDesc()
		{
			VkVertexInputBindingDescription bindingDesc;
			bindingDesc.binding = 0;
			bindingDesc.stride = m_shaderData.InAttri.Size;
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDesc;
		}

		std::vector<VkVertexInputAttributeDescription> ShaderModuleBase::GetAttributes()
		{
			std::vector<VkVertexInputAttributeDescription> attributes;

			for (auto it = m_shaderData.InAttri.Attributes.begin(); it != m_shaderData.InAttri.Attributes.end(); ++it)
			{
				VkVertexInputAttributeDescription attri;
				attri.binding = 0;
				attri.location = (*it).Location;
				attri.format = GetVertexFormat((*it).Type);
				attri.offset = (*it).Stride;

				attributes.push_back(attri);
			}

			return attributes;
		}

		VkFormat ShaderModuleBase::GetVertexFormat(const ShaderAttributeType& type)
		{
			switch (type)
			{
				case ShaderAttributeType::Int: return VK_FORMAT_R8_UINT;
				case ShaderAttributeType::Float: return VK_FORMAT_R16_SFLOAT;

				case ShaderAttributeType::Vec2: return VK_FORMAT_R32G32_SFLOAT;
				case ShaderAttributeType::Vec3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ShaderAttributeType::Vec4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		}

		const std::vector<uint32_t> ShaderModuleBase::CompileGLSL(const std::string& filePath)
		{
			std::ifstream file(filePath);
			if (file.is_open())
			{
					std::string InputGLSL((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					const char* InputCString = InputGLSL.c_str();

					return ComplieToRaw(filePath, GetShaderStage(GetSuffix(filePath)), InputCString);
			}
			return std::vector<uint32_t>();
		}

		std::string ShaderModuleBase::PreprocessShader(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize)
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;

			// Like -DMY_DEFINE=1
			options.AddMacroDefinition("MY_DEFINE", "1");
			if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

			shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(
				source, kind, source_name.c_str(), options);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) 
			{
				std::cerr << result.GetErrorMessage();
				return "";
			}

			return { result.cbegin(), result.cend() };
		}

		std::vector<uint32_t> ShaderModuleBase::ComplieToRaw(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize)
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;

			// Like -DMY_DEFINE=1
			options.AddMacroDefinition("MY_DEFINE", "1");
			if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

			shaderc::SpvCompilationResult module =
				compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				std::cerr << module.GetErrorMessage();
				return std::vector<uint32_t>();
			}

			return { module.cbegin(), module.cend() };
		}

		std::string ShaderModuleBase::GetFilePath(const std::string& filePath)
		{
			size_t found = filePath.find_last_of("/\\");
			return filePath.substr(0, found);
		}

		std::string ShaderModuleBase::GetSuffix(const std::string& name)
		{
			const size_t pos = name.rfind('.');
			return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
		}

		shaderc_shader_kind ShaderModuleBase::GetShaderStage(const std::string& stage)
		{
			if (stage == "vert") 
			{
				return shaderc_shader_kind::shaderc_glsl_vertex_shader;
			}
			else if (stage == "tesc") 
			{
				return shaderc_shader_kind::shaderc_glsl_tess_control_shader;
			}
			else if (stage == "tese")
			{
				return shaderc_shader_kind::shaderc_glsl_tess_evaluation_shader;
			}
			else if (stage == "geom") 
			{
				return shaderc_shader_kind::shaderc_glsl_geometry_shader;
			}
			else if (stage == "frag") 
			{
				return shaderc_shader_kind::shaderc_glsl_fragment_shader;
			}
			else if (stage == "comp") 
			{
				return shaderc_shader_kind::shaderc_glsl_compute_shader;
			}
		}
	}
}
