#pragma once

#include "Engine/Graphics/GPUResource.h"

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
		Free_Descriptor_Set = 1 << 0,
		Update_After_Bind = 1 << 1,
		Update_After_Bind_EXT = Update_After_Bind,
	};

	struct GPUDescriptorSetDesc
	{
		GPUDescriptorSetDesc(GPUDescriptorPool* pool, GPUPipeline* pipeline)
			: Pool(pool), m_pipeline(pipeline)
		{ }
		GPUDescriptorPool* Pool;
		GPUPipeline* m_pipeline;
	};

	class GPUDescriptorSet : public GPUResource
	{
	public:
		GPUDescriptorSet();
		virtual ~GPUDescriptorSet();

		static GPUDescriptorSet* New();

		const GPUDescriptorSetDesc& GetDesc() const { return m_desc; }
		virtual void Init(GPUDescriptorSetDesc& desc) = 0;

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

		virtual void Init() = 0;

		virtual GPUDescriptorSet* AllocaSet(GPUDescriptorSetDesc& desc) = 0;
		virtual void ResetPool(GPUDescriptorPoolResetFlag resetFlags) = 0;

	protected:
		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::Descriptor; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }
		
	protected:
		std::vector<GPUDescriptorSet*> m_sets;
	};
}