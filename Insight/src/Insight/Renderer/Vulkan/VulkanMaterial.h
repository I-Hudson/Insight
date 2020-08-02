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
			void* DataMapped;
			VkBuffer Buffer;
			VkDeviceMemory BufferMem;
			int Size;
			int Binding;
			int TypeSize;
			VkDescriptorType Type;
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
			template<typename T>
			void UpdateDynamicUniforms(const std::string& key, void* uniformData, int numOfObjects, int binding);

			void Resize();
			void Bind(CommandBuffer* commandBuffers, int drawIndex);

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

			int m_dynamicOffset = 0;

			VkBuffer m_uniformBuffers = VK_NULL_HANDLE;
			VkDeviceMemory m_uniformBuffersMem = VK_NULL_HANDLE;

			std::unordered_map<std::string, UniformData> m_uniformData;
			std::unordered_map<std::string, SamplerData> m_samplerData;

			ModelUniformBuffer m_modelUniform;

			static VulkanRenderer* s_Renderer;
			friend VulkanRenderer;
		};

		template<typename T>
		void* alignedAlloc(size_t size, size_t alignment)
		{
			void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
			data = _aligned_malloc(size, alignment);
#else
			int res = posix_memalign(&data, alignment, size);
			if (res != 0)
				data = nullptr;
#endif
			return data;
		}

		template<typename T>
		inline void VulkanMaterial::UpdateDynamicUniforms(const std::string& key, void* uniformData, int numOfObjects, int binding)
		{
			UniformData data;
			if (m_uniformData.find(key) == m_uniformData.end())
			{
				// Calculate required alignment based on minimum device offset alignment
				size_t minUboAlignment = s_Renderer->GetDeviceWrapper()->GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
				int dynamicAlignment = sizeof(T);
				if (minUboAlignment > 0)
				{
					dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
				}

				data.TypeSize = sizeof(T);
				data.Size = data.TypeSize * numOfObjects;
				data.Binding = binding;
				data.Type = data.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				CreateUniformBufferMem(data);
				MapBufferMem(data);

				m_dynamicOffset = dynamicAlignment;

				m_uniformData[key] = data;
			}
			data = m_uniformData[key];

			memcpy(data.DataMapped, uniformData, data.Size);

			VkMappedMemoryRange mappedMemoryRange{};
			mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

			// Flush to make changes visible to the host
			VkMappedMemoryRange memoryRange = mappedMemoryRange;
			memoryRange.memory = data.BufferMem;
			memoryRange.size = data.Size;
			vkFlushMappedMemoryRanges(s_Renderer->GetDevice(), 1, &memoryRange);
		}
	}
}
#endif