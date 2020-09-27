#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Insight/Renderer/Material.h"
#include "Vulkan.h"
#include "VulkanShader.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "CommandBuffer.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct VKMaterialRenderData : public MaterialRenderData
{
	int PositionDynamicUniformOffset;
};

namespace Platform
{
	class VulkanRenderer;

	struct UniformData
	{
		void* DataMapped;
		VkBuffer Buffer;
		VkDeviceMemory BufferMem;
		int Size;
		int TypeSize;
		int Binding;
		VkDescriptorType Type;

		UniformData()
			: DataMapped(nullptr)
			, Buffer(VK_NULL_HANDLE)
			, BufferMem(VK_NULL_HANDLE)
			, Size(0)
			, TypeSize(0)
			, Binding(0)
			, Type(VK_DESCRIPTOR_TYPE_SAMPLER)
		{ }
	};

	struct UniformObjectsData : UniformData
	{
		void* Data;
		int Offset;
		std::vector<MeshComponent*> Owners;
	};

	struct UniformDyanmicData : UniformData
	{
		void* Data;
		std::vector<void*> Owners;
	};

	struct UniformDynamicDataContainer : UniformData
	{
		std::vector<UniformObjectsData> Positions;
	};

	struct SamplerData : UniformData
	{
		VkImageView* ImageView;
		VkSampler* Sampler;
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

		virtual void SetShader(Insight::Render::Shader* shader) override;
		virtual Insight::Render::Shader* GetShader() override;
		virtual void SetUniforms() override;
		virtual void UpdateMVPUniform(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model = glm::mat4()) override;
		void UpdateObjectsUniforms();

		virtual void UpdateUniform(const std::string& key, void* uniformData, size_t size, int binding) override;
		virtual void UpdateSampler2D(const std::string& key, void* imageView, void* sampler, int binding) override;

		virtual MaterialRenderData IncrementUsageCount(const MeshComponent* meshComponent) override;
		virtual void DecrementUsageCount(const MeshComponent* meshComponent) override;

		void Resize();
		void Bind(CommandBuffer* commandBuffers, const MeshComponent* meshBeingDrawn);

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

		bool m_hasDynamicUniform = false;
		int m_minDynamicOffset = 0;

		VkBuffer m_uniformBuffers = VK_NULL_HANDLE;
		VkDeviceMemory m_uniformBuffersMem = VK_NULL_HANDLE;

		std::unordered_map<std::string, UniformData> m_uniformData;
		UniformDynamicDataContainer m_uniformObjectsData;
		std::unordered_map<std::string, UniformDyanmicData> m_uniformDynamicData;
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
}
#endif