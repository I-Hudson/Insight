#pragma once

#include "Graphics/GPU/GPUBuffer.h"
#include "GPUDevice_Vulkan.h"

struct VmaAllocation_T;

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUBuffer_Vulkan : public GPUBuffer, public GPUResource_Vulkan
			{
			public:
				GPUBuffer_Vulkan();
				virtual ~GPUBuffer_Vulkan() override;

				vk::Buffer GetBuffer() const { return m_buffer; }

			protected:
				virtual void Create(GPUBufferCreateInfo desc) override;
				virtual void Destroy() override;
				virtual bool Upload(const void* data, u64 size, u64 offset) override;

			private:
				vk::Buffer m_buffer{ nullptr };
				VmaAllocation_T* m_vmaAllocation{ nullptr};
			};
		}
	}
}