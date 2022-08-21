#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/PipelineStateObject.h"

#include <unordered_set>
#include <queue>

#include <vulkan/vulkan.hpp>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;
			class DescriptorPoolPage_Vulkan;

			class RHI_DescriptorLayout_Vulkan : public RHI_DescriptorLayout
			{
			public:
				vk::DescriptorSetLayout GetLayout() const { return m_layout; }

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			protected:
				virtual void Create(RenderContext* context, int set, std::vector<Descriptor> descriptors) override;

			private:
				RenderContext_Vulkan* m_context{ nullptr };
				vk::DescriptorSetLayout m_layout;
			};

			class RHI_Descriptor_Vulkan : public RHI_Descriptor
			{
			public:

				virtual void Update(const std::vector<Descriptor>& descriptors) override;
				virtual u64 GetHash(bool includeResouce = false) override;

				vk::DescriptorSet GetSet() const { return m_set; }

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			private:
				RenderContext_Vulkan* m_context{ nullptr };
				vk::DescriptorSet m_set;
				DescriptorPoolPage_Vulkan* m_pool{ nullptr };

				u64 m_hash = 0;
				u64 m_hashWithResouce = 0;

				friend class DescriptorPoolPage_Vulkan;
			};

			class DescriptorPoolPage_Vulkan
			{
			public:
				DescriptorPoolPage_Vulkan();
				DescriptorPoolPage_Vulkan(RenderContext_Vulkan* context, int capacity);

				RHI_Descriptor* GetDescriptorSet(const std::vector<Descriptor>& descriptors);

				bool HasAllocatedDescriptor(const std::vector<Descriptor>& descriptors);
				bool HasFreeDescriptor(const std::vector<Descriptor>& descriptors);
				void FreeDescriptor(RHI_Descriptor* descriptor);

				vk::DescriptorPool GetPool() const { return m_descriptorPool; }

				void Reset();
				void Release();

				u64 GetSize() const { return m_size; }
				u64 GetCapacity() const { return m_capacity; }
				bool IsFull() const { return GetSize() == GetCapacity(); }

			private:
				RenderContext_Vulkan* m_context = nullptr;
				vk::DescriptorPool m_descriptorPool;
				u32 m_size = 0;
				u32 m_capacity = 0;
				
				std::unordered_map<u64, RHI_Descriptor*> m_usedDescriptors;
				std::unordered_map<u64, RHI_Descriptor*> m_freeDescriptors;
				std::unordered_map<RHI_Descriptor*, u64> m_descriptorToHash;
			};

			class DescriptorPool_Vulkan
			{
			public:

				void Create(RenderContext_Vulkan* context);
				
				RHI_Descriptor* GetDescriptor(const std::vector<Descriptor>& descriptors);
				void FreeDescriptor(RHI_Descriptor* descriptor);

				void Reset();
				void Destroy();

			private:
				void AddNewPool(u64 hash);

			private:
				RenderContext_Vulkan* m_context = nullptr;
				std::unordered_map<u64, DescriptorPoolPage_Vulkan> m_pools;
			};

			//class DescriptorAllocator_Vulkan : public DescriptorAllocator
			//{
			//public:
			//	virtual void SetRenderContext(RenderContext* context) override;
			//	virtual bool GetDescriptors(std::vector<RHI_Descriptor*>& descriptors) override;

			//	virtual void Reset() override;
			//	virtual void Destroy() override;

			//private:
			//	RenderContext_Vulkan* m_context = nullptr;
			//	DescriptorPool_Vulkan m_pool;
			//};
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)