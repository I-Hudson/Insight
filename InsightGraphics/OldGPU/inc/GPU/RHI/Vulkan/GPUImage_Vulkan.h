#pragma once

#include "Graphics/GPU/GPUImage.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"

struct VmaAllocation_T;

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUImage_Vulkan : public GPUImage, public GPUResource_Vulkan
			{
			public:
				virtual ~GPUImage_Vulkan() override;

				virtual void LoadFromData(GPUImageCreateInfo info) override;

			private:
				virtual void Destroy() override;

			private:
				vk::Image m_image{ nullptr };
				vk::ImageView m_view{ nullptr };
				VmaAllocation_T* m_vmaAllocation{ nullptr };
			};
		}
	}
}