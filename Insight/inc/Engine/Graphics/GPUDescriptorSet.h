#pragma once

#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"

namespace Insight::Graphics
{
	const u32 DESCRIPTOR_POOL_SIZE = 256;
	const u32 DESCRIPTOR_POOL_UNIFORM_BUFFER_COUNT = 15;
	const u32 DESCRIPTOR_POOL_UNIFORM_DYNAMIC_BUFFER_COUNT = 6;
	const u32 DESCRIPTOR_POOL_SAMPLER_BUFFER_COUNT = 16;

	class GPUPipeline;
	class GPUDescriptorPool;
	class GPUImage;
	class GPUBuffer;

	enum GPUDescriptorPoolResetFlag
	{
		None = 0,
		Free_Descriptor_Set = 1 << 0,
		Update_After_Bind = 1 << 1,
		Update_After_Bind_EXT = Update_After_Bind,
	};

	struct GPUDescriptorSetDesc
	{
		GPUDescriptorSetDesc(GPUDescriptorPool* pool, void* layout)
			: Pool(pool), Layout(layout)
		{ }
		GPUDescriptorPool* Pool;
		void* Layout;
	};

	class GPUDescriptorSet : public GPUResource
	{
	public:
		GPUDescriptorSet();
		virtual ~GPUDescriptorSet();

		static GPUDescriptorSet* New();

		const GPUDescriptorSetDesc& GetDesc() const { return m_desc; }
		virtual GPUResults Init(GPUDescriptorSetDesc& desc) = 0;

		virtual void BindTexture(GPUImage* image, u32 slot) = 0;
		virtual void BindUniformBuffer(GPUBuffer* buffer, u32 slot) = 0;
		virtual void BindDynamicUniformBuffer(GPUBuffer* buffer, u32 slot) = 0;

	protected:
		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Descriptor; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }
	
	protected:
		GPUDescriptorPool* m_pool;
		GPUDescriptorSetDesc m_desc;
	};

	class GPUDescriptorPool : public GPUResource
	{
	public:
		GPUDescriptorPool();
		virtual ~GPUDescriptorPool();

		static GPUDescriptorPool* New();

		virtual GPUResults Init() = 0;

		virtual GPUDescriptorSet* AllocaSet(GPUDescriptorSetDesc& desc) = 0;
		virtual void ResetPool(GPUDescriptorPoolResetFlag resetFlags) = 0;

	protected:
		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Descriptor; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }
		
	protected:
		std::vector<GPUDescriptorSet*> m_sets;
	};

	class GPUDescriptorAllocator 
	{
	public:
		virtual ~GPUDescriptorAllocator() { }
		static GPUDescriptorAllocator* New();
		virtual void Init() = 0;
		virtual void ResetPools() = 0;
	};
	class GPUDescriptorLayoutCache 
	{
	public:
		virtual ~GPUDescriptorLayoutCache() { }
		static GPUDescriptorLayoutCache* New();
	};

	class GPUDescriptorBuilder
	{
	public:
		static GPUDescriptorBuilder* New();

		virtual GPUDescriptorBuilder* Begin(GPUDescriptorLayoutCache* layoutCache, GPUDescriptorAllocator* allocator) = 0;

		virtual GPUDescriptorBuilder* BindBuffer(u32 binding, GPUBuffer* buffer, DescriptorType type, ShaderStage stage) = 0;
		virtual GPUDescriptorBuilder* BindImage(u32 binding, GPUImage* image, DescriptorType type, ShaderStage stage) = 0;
		virtual GPUDescriptorBuilder* BindImageArray(u32 binding, std::vector<GPUImage*>& images, DescriptorType type, ShaderStage stage) = 0;
	
		virtual bool Build(GPUDescriptorSet*& set) = 0;
	};
}