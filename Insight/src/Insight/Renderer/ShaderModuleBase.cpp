#include "ispch.h"
#include "ShaderModuleBase.h"
#include "Insight/ShaderParser/ShaderParser.h"

#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/Device.h"
#elif defined(IS_OPENGL)
#endif

namespace Insight
{
	namespace Render
	{
#ifdef IS_VULKAN
		ShaderModuleBase::ShaderModuleBase(const Device* device, const std::string& filepath)
			: m_device(device), m_type(GetShaderTypeFromPath(GetSuffix(filepath))), 
			m_shaderModule(0)
		{
			m_shaderData = ShaderParser::ParseShader(filepath, m_type);


			auto spirvData = CompileGLSL(filepath);
			VkShaderModuleCreateInfo createInfo = VulkanInits::ShaderModuleInfo(spirvData);
			ThrowIfFailed(vkCreateShaderModule(device->GetDevice(), &createInfo, nullptr, &m_shaderModule));
		}

		ShaderModuleBase::ShaderModuleBase(const Device* device, const std::string& filepath, const ShaderType& type)
			: m_device(device), m_type(type), 
			m_shaderModule(0)
		{
			m_shaderData = ShaderParser::ParseShader(filepath, type);

			auto spirvData = CompileGLSL(filepath);
			VkShaderModuleCreateInfo createInfo = VulkanInits::ShaderModuleInfo(spirvData);
			ThrowIfFailed(vkCreateShaderModule(device->GetDevice(), &createInfo, nullptr, &m_shaderModule));
		}
#elif defined(IS_OPENGL)
		ShaderModuleBase::ShaderModuleBase(const std::string& filepath)
			: m_type(GetShaderTypeFromPath(GetSuffix(filepath)))
		{
			m_shaderData = ShaderParser::ParseShader(filepath, m_type);
			GetSource(filepath);
		}

		ShaderModuleBase::ShaderModuleBase(const std::string& filepath, const ShaderType& type)
			: m_type(type)
		{
			m_shaderData = ShaderParser::ParseShader(filepath, type);
			GetSource(filepath);
		}
#endif

		ShaderModuleBase::~ShaderModuleBase()
		{ }

		void ShaderModuleBase::Destroy()
		{
#ifdef IS_VULKAN
			vkDestroyShaderModule(m_device->GetDevice(), m_shaderModule, nullptr);
#endif
		}

		std::string ShaderModuleBase::GetSource(const std::string& filePath)
		{
			std::ifstream file(filePath);
			if (file.is_open())
			{
				std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();

				m_shaderData.RawSource = source;

				return source;
			}
			return std::string();
		}

#ifdef IS_VULKAN
		VkShaderStageFlagBits ShaderModuleBase::GetShaderStageBit() const
		{
			switch (m_type)
			{
				case Insight::Render::None: break;

				case Insight::Render::VertexShader: return VK_SHADER_STAGE_VERTEX_BIT;
				case Insight::Render::GeometryShader: return VK_SHADER_STAGE_GEOMETRY_BIT;
				case Insight::Render::FragmentShader: return VK_SHADER_STAGE_FRAGMENT_BIT;
				case Insight::Render::ComputeShader: return VK_SHADER_STAGE_COMPUTE_BIT;
			}
		}

		VkDescriptorType ShaderModuleBase::GetShaderDescriptorType(const ShaderAttributeType& type)
		{
			switch (type)
			{
				case Insight::Render::ShaderAttributeType::Sampler2D: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				default: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
		}

		std::vector<VkDescriptorSetLayoutBinding> ShaderModuleBase::GetPipelineLayoutCreateInfo()
		{
			std::vector<VkDescriptorSetLayoutBinding> createInfos{};
			for (auto it = m_shaderData.UniformBlocks.begin(); it != m_shaderData.UniformBlocks.end(); ++it)
			{
				auto desc = VulkanInits::DescriptorSetLayoutBinding();
				desc.binding = (*it).Binding;
				desc.descriptorType = GetShaderDescriptorType((*it).Type);;
				desc.descriptorCount = 1;
				desc.stageFlags = GetShaderStageBit();
				createInfos.push_back(desc);
			}

			return createInfos;
		}

		std::vector<VkVertexInputBindingDescription> ShaderModuleBase::GetVertexBindingDesc()
		{
			std::vector<VkVertexInputBindingDescription> inputDescs;

			if (m_shaderData.InAttri.Size > 0)
			{
				VkVertexInputBindingDescription bindingDesc;
				bindingDesc.binding = 0;
				bindingDesc.stride = m_shaderData.InAttri.Size;
				bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				inputDescs.push_back(bindingDesc);
			}

			return inputDescs;
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

		const std::vector<uint32_t> ShaderModuleBase::CompileGLSL(const std::string& filePath)
		{
			return ComplieToRaw(filePath, GetShaderStage(GetSuffix(filePath)), GetSource(filePath));
		}

#elif defined(IS_OPENGL)
#endif

		ShaderType ShaderModuleBase::GetShaderType() const
		{
			return m_type;
		}

		ShaderType ShaderModuleBase::GetShaderTypeFromPath(const std::string& string)
		{
			if (string == "vert")
			{
				return ShaderType::VertexShader;
			}
			else if (string == "tesc")
			{
				return ShaderType::VertexShader;
			}
			else if (string == "tese")
			{
				return ShaderType::VertexShader;
			}
			else if (string == "geom")
			{
				return ShaderType::GeometryShader;
			}
			else if (string == "frag")
			{
				return ShaderType::FragmentShader;
			}
			else if (string == "comp")
			{
				return ShaderType::ComputeShader;
			}
			return ShaderType::None;
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
	}
}
