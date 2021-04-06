#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Graphics/Texture.h"
#include "VulkanHeader.h"

namespace vks
{
	class IS_API VulkanTexture : public Render::Texture
	{
	public:
		VulkanTexture(); 
		virtual ~VulkanTexture() override;

		virtual bool IsValid() override;

		VkImage& GetImage() { return m_image; }
		VkImageView& GetImageView() { return m_imageView; }
		VkSampler& GetSampler() { return m_sampler; }
		VkImageLayout& GetImageLayout() { return m_imageLayout; }
		VkDeviceMemory& GetDeviceMemory() { return m_deviceMemory; }

	protected:
		virtual void CreateGPUResource() override;

	private:
		VkImage m_image;
		VkImageView m_imageView;
		VkSampler m_sampler;
		VkDeviceMemory m_deviceMemory;
		VkImageLayout m_imageLayout;
	};
}
