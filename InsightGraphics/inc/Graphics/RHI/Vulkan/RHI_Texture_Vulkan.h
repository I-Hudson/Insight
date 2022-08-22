#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RHI_Texture_Vulkan : public RHI_Texture
			{
			public:
				virtual ~RHI_Texture_Vulkan() override;

				// RHI_Texture
				virtual void Create(RenderContext* context, RHI_TextureCreateInfo createInfo) override;
				virtual void Upload(void* data, int sizeInBytes) override;
				virtual std::vector<Byte> Download(void* data, int sizeInBytes) override;

				// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

				vk::Image GetImage() const { return m_image; }
				vk::ImageView GetImageView() const { return m_imageView; }

			private:
				RenderContext_Vulkan* m_context = nullptr;
				vk::Image m_image{ nullptr };
				vk::ImageView m_imageView{ nullptr };
				VmaAllocation m_imageAllocation = nullptr;

				friend class RenderContext_Vulkan;
			};
		}
	}
}

#endif // defined(IS_VULKAN_ENABLED)