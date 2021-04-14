#pragma once
#include "Engine/GraphicsAPI/Vulkan/GPUDescriptorSetVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUDescriptorAllocatorVulkan : public Graphics::GPUDescriptorAllocator
	{
	public:
		GPUDescriptorAllocatorVulkan();
		~GPUDescriptorAllocatorVulkan();

		void Init();
		bool Allocate(Graphics::GPUDescriptorSet* set, VkDescriptorSetLayout layout);
		void ResetPools();

	private:
		Graphics::GPUDescriptorPool* GrabPool();
		Graphics::GPUDescriptorPool* CreatePool();

	private:
		Graphics::GPUDescriptorPool* m_currentPool;
		std::vector<Graphics::GPUDescriptorPool*> m_usedPools;
		std::vector<Graphics::GPUDescriptorPool*> m_freePools;
	};

	class GPUDescriptorLayoutCacheVulkan : public Graphics::GPUDescriptorLayoutCache
	{
	public:
		void Init();
		void Cleanup();

		VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo
		{
			std::vector<VkDescriptorSetLayoutBinding> Bindings;

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

		GPUDescriptorBuilder* BindBuffer(u32 binding, Graphics::GPUBuffer* buffer, DescriptorType type, ShaderStage stage) override;
		GPUDescriptorBuilder* BindImage(u32 binding, Graphics::GPUImage* image, DescriptorType type, ShaderStage stage) override;

		virtual bool Build(Graphics::GPUDescriptorSet* set) override;

	private:
		std::vector<VkWriteDescriptorSet> m_writes;
		std::vector<VkDescriptorSetLayoutBinding> m_bindings;

		GPUDescriptorLayoutCacheVulkan* m_layoutCache;
		GPUDescriptorAllocatorVulkan* m_allocator;
	};
}