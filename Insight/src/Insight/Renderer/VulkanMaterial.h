#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Shader.h"
#include "Insight/Renderer/Material.h"

#include "Insight/Renderer/Lowlevel/DescriptorPool.h"
#include "Insight/Renderer/Lowlevel/DescriptorSet.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"

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
			int Index;
		};

		struct MVPUniformBuffer
		{
			glm::mat4 u_model;
			glm::mat4 u_view;
			glm::mat4 u_proj;
		};

		class VulkanMaterial : public Material
		{
		public:
			VulkanMaterial();
			virtual ~VulkanMaterial() override;

			virtual void SetShader(Shader* shader) override;
			virtual const Shader* GetShader() override;
			virtual void UpdateUniforms() override;
			virtual void UpdateLoadUniforms() override;
			virtual void UpdateLoadUniforms(const std::string& key, void* uniformData, size_t size, int binding, int index) override;

			void Bind(CommandBuffer* commandBuffers);

		private:
			void DestroyUniformBuffers();
			void MapNewBufferMem(UniformData& uniformData);

		private:
			Shader* m_shader;

			DescriptorPool* m_descPool;
			DescriptorSet* m_descSet;
			bool m_updatedDesc = false;

			VkBuffer m_uniformBuffers = VK_NULL_HANDLE;
			VkDeviceMemory m_uniformBuffersMem = VK_NULL_HANDLE;

			std::unordered_map<std::string, UniformData> m_uniformData;

			static VulkanRenderer* s_Renderer;
			friend VulkanRenderer;
		};
	}
}