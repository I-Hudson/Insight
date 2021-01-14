#pragma once
#include "Insight/Core/Core.h"
#include "Renderer/Texture.h"
#include "VulkanHeader.h"

namespace vks
{
	class IS_API VulkanTextureGPUData : public Insight::Render::TextureGPUData
	{
	public:
		virtual ~VulkanTextureGPUData() override;

		virtual void Init(void* textureData, const U32& textureDataSize, const U32& width, const U32& height, const U32& channels) override;

		VkImage GetImage() const { return m_image; }
		VkImageView GetImageView() const { return m_imageView; }
		VkSampler GetSampler() const { return m_sampler; }
		VkImageLayout GetImageLayout() const { return m_imageLayout; }

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
