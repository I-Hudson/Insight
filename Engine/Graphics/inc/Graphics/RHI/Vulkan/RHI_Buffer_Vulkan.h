#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

struct VmaAllocation_T;

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;

			class RHI_Buffer_Vulkan : public RHI_Buffer
			{
			public:
				virtual ~RHI_Buffer_Vulkan() override;

				virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes, u64 stride, RHI_Buffer_Overrides overrides) override;
				virtual RHI_BufferView Upload(const void* data, u64 sizeInBytes, u64 offset, u64 alignment) override;
				virtual std::vector<Byte> Download() override;
				virtual void Resize(u64 newSizeBytes) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::string name) override;

				VkBuffer GetBuffer() const { return m_buffer; }

			private:
				RenderContext_Vulkan* m_context = nullptr;
				void* m_mappedData = nullptr;
				VkBuffer m_buffer;
				VmaAllocation_T* m_vmaAllocation;
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)