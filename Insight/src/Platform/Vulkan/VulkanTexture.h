#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Graphics/Texture.h"
#include "VulkanHeader.h"

namespace vks
{
	class IS_API VulkanTextureGPUData : public Render::TextureGPUData
	{
	public:
		virtual ~VulkanTextureGPUData() override;

		virtual void Init(void* textureData, const U32& textureDataSize, const U32& width, const U32& height, const U32& channels) override;

		VkImage& GetImage() { return m_image; }
		VkImageView& GetImageView() { return m_imageView; }
		VkSampler& GetSampler() { return m_sampler; }
		VkImageLayout& GetImageLayout() { return m_imageLayout; }
		VkDeviceMemory& GetDeviceMemory() { return m_deviceMemory; }

	private:
		VkImage m_image;
		VkImageView m_imageView;
		VkSampler m_sampler;
		VkDeviceMemory m_deviceMemory;

		VkImageLayout m_imageLayout;
		U32 m_width, m_height;
		U32 m_mipLevels;
	};
}
