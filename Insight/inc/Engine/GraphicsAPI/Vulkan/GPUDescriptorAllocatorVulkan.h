#pragma once
#include "Engine/GraphicsAPI/Vulkan/GPUDescriptorSetVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUDescriptorAllocatorVulkan : public Graphics::GPUDescriptorAllocator
	{
	public:
		GPUDescriptorAllocatorVulkan();
		virtual ~GPUDescriptorAllocatorVulkan() override;

		virtual void Init() override;
		bool Allocate(Graphics::GPUDescriptorSet*& set, VkDescriptorSetLayout layout);
		virtual void ResetPools() override;

	private:
		Graphics::GPUDescriptorPool* GrabPool();
		Graphics::GPUDescriptorPool* CreatePool();

	private:
		Graphics::GPUDescriptorPool* m_currentPool;
		std::vector<Graphics::GPUDescriptorPool*> m_usedPools;
		std::vector<Graphics::GPUDescriptorPool*> m_freePools;
		std::unordered_map<u64, std::vector<Graphics::GPUDescriptorSet*>> m_freeSets;
		std::vector<Graphics::GPUDescriptorSet*> m_allocatedSets;
	};

	class GPUDescriptorLayoutCacheVulkan : public Graphics::GPUDescriptorLayoutCache
	{
	public:
		virtual ~GPUDescriptorLayoutCacheVulkan() override;

		void Init();
		void Cleanup();

		VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo
		{
			std::vector<VkDescriptorSetLayoutBinding> Bindings;
			std::vector<VkDescriptorBindingFlags> BindingsFlags;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t hash() const;
		};

	private:
		struct DescriptorLayoutHash {
			std::size_t operator()(const DescriptorLayoutInfo& k) const 
			{
				return k.hash();
			}
		};

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_layoutCache;
	};

	class GPUDescriptorBuilderVulkan : public Graphics::GPUDescriptorBuilder
	{
	public:
		virtual GPUDescriptorBuilder* Begin(Graphics::GPUDescriptorLayoutCache* layoutCache, Graphics::GPUDescriptorAllocator* allocator) override;

		virtual GPUDescriptorBuilder* BindBuffer(u32 binding, Graphics::GPUBuffer* buffer, DescriptorType type, ShaderStage stage) override;
		virtual GPUDescriptorBuilder* BindImage(u32 binding, Graphics::GPUImage* image, DescriptorType type, ShaderStage stage) override;
		virtual GPUDescriptorBuilder* BindImageArray(u32 binding, std::vector<Graphics::GPUImage*>& images, DescriptorType type, ShaderStage stage) override;

		virtual bool Build(Graphics::GPUDescriptorSet*& set) override;

	private:
		std::vector<VkWriteDescriptorSet> m_writes;
		std::vector<VkDescriptorSetLayoutBinding> m_bindings;

		struct DiscriptorTextureArray
		{
			std::vector<VkDescriptorImageInfo> ImageInfos;
		};
		std::vector<DiscriptorTextureArray> m_tempTextureArrays;

		GPUDescriptorLayoutCacheVulkan* m_layoutCache;
		GPUDescriptorAllocatorVulkan* m_allocator;
	};
}