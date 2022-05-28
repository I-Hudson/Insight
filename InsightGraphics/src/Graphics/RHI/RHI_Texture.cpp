#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/GraphicsManager.h"

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
#if defined(TRUE)
			if (GraphicsManager::IsVulkan()) { return nullptr; }
#elif defined(IS_DX12_ENABLED)
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_Texture_DX12); }
#endif	
			return nullptr;
		}

		void RHI_Texture::LoadFromFile(std::string filePath)
		{
			if (filePath.empty() || !std::filesystem::exists(filePath))
			{
				return;
			}

			int width, height, channels;
			Byte* data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if (!data)
			{
				return;
			}

			Create(GraphicsManager::Instance().GetRenderContext(), TextureType::Tex2D, width, height, channels);
			const u64 textureSize = width * height * channels;
			Upload(data, textureSize);

			stbi_image_free(data);
		}
	}
}