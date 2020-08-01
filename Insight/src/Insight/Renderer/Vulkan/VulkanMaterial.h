#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

#include "Insight/Renderer/Material.h"
#include "Insight/Renderer/Vulkan/VulkanShader.h"

#include "Insight/Renderer/Vulkan/DescriptorPool.h"
#include "Insight/Renderer/Vulkan/DescriptorSet.h"
#include "Insight/Renderer/Vulkan/CommandBuffer.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

namespace Insight
{
	namespace Render
	{
		class VulkanRenderer;

		struct UniformData
		{
			void* Data;
			VkBuffer Buffer;
			VkDeviceMemory BufferMem;
			int Size;
			int Binding;
		};

		struct SamplerData
		{
			VkImageView* ImageView;
			VkSampler* Sampler;
			int Binding;
		};

		struct MVPUniformBuffer
		{
			glm::mat4 u_view;
			glm::mat4 u_proj;
		};

		struct ModelUniformBuffer
		{
			glm::mat4* u_model = nullptr;
		};

		class VulkanMaterial : public Material
		{
		public:
			VulkanMaterial();
			virtual ~VulkanMaterial() override;

			virtual void SetShader(Shader* shader) override;
			virtual Shader* GetShader() override;
			virtual void SetUniforms() override;
			virtual void UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model) override;
			virtual void UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding) override;
			virtual void UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding) override;

			void Resize();
			void Bind(CommandBuffer* commandBuffers);

			DescriptorPool* GetDescPool() { return m_descPool; }

		private:
			void DestroyUniformBuffers();
			void CreateUniformBufferMem(UniformData& uniformData);
			void MapBufferMem(UniformData& uniformData);
			void UnMapBufferMem(UniformData& uniformData);

		private:
			VulkanShader* m_shader;

			DescriptorPool* m_descPool;
			DescriptorSet* m_descSet;
			bool m_updatedDesc = false;

			VkBuffer m_uniformBuffers = VK_NULL_HANDLE;
			VkDeviceMemory m_uniformBuffersMem = VK_NULL_HANDLE;

			std::unordered_map<std::string, UniformData> m_uniformData;
			std::unordered_map<std::string, SamplerData> m_samplerData;

			ModelUniformBuffer m_modelUniform;

			static VulkanRenderer* s_Renderer;
			friend VulkanRenderer;
		};
	}
}
#endif