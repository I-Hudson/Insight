#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"

#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_Texture* RHI_Texture::New()
		{
#if defined(IS_VULKAN_ENABLED)
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_Texture_Vulkan); }
#endif
#if defined(IS_DX12_ENABLED)
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_Texture_DX12); }
#endif	
			return nullptr;
		}

		void RHI_Texture::LoadFromFile(std::string filePath)
		{
			IS_PROFILE_FUNCTION();
			std::unique_lock lock(m_mutex);
			if (filePath.empty() || !std::filesystem::exists(filePath))
			{
				return;
			}
			lock.unlock();

			int width, height, channels;
			Byte* data = nullptr;
			{
				IS_PROFILE_SCOPE("stbi_load");
				data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			}
			if (!data)
			{
				return;
			}

			TrackPtr(data);
			RHI_TextureInfo createInfo = { };
			createInfo.TextureType = TextureType::Tex2D;
			createInfo.Width = width;
			createInfo.Height = height;
			createInfo.Depth = 1;
			createInfo.Format = PixelFormat::R8G8B8A8_UNorm;
			createInfo.ImageUsage = ImageUsageFlagsBits::Sampled | ImageUsageFlagsBits::TransferDst;

//#define RHI_TEXTURE_DEFER_ENABLED
#ifdef RHI_TEXTURE_DEFER_ENABLED
			GraphicsManager::Instance().GetRenderContext()->GetDeferredManager().Push([this, createInfo, width, height, data](RHI_CommandList* cmdList)
				{
					Create(GraphicsManager::Instance().GetRenderContext(), createInfo);
					const u64 textureSize = width * height * STBI_rgb_alpha;
					Upload(data, (int)textureSize, cmdList);

					stbi_image_free(data);
					UntrackPtr(data);
				});
#else
			const u64 textureSize = width * height * STBI_rgb_alpha;
			Upload(data, (int)textureSize);
			
			stbi_image_free(data);
			UntrackPtr(data);
#endif
		}

		void RHI_Texture::LoadFromData(Byte* data, u32 width, u32 height, u32 depth, u32 channels)
		{
			const u64 size_in_bytes = width * height * depth * channels;
			if (data == nullptr || size_in_bytes == 0)
			{
				return;
			}

			RHI_TextureInfo createInfo = { };
			createInfo.TextureType = TextureType::Tex2D;
			createInfo.Width = width;
			createInfo.Height = height;
			createInfo.Depth = depth;
			createInfo.Format = PixelFormat::R8G8B8A8_UNorm;
			createInfo.ImageUsage = ImageUsageFlagsBits::Sampled | ImageUsageFlagsBits::TransferDst;

			Create(GraphicsManager::Instance().GetRenderContext(), createInfo);
			Upload(data, static_cast<int>(size_in_bytes));
		}

		void RHI_Texture::Upload(void* data, int sizeInBytes)
		{
			Upload(data, sizeInBytes, nullptr);
		}
	}
}