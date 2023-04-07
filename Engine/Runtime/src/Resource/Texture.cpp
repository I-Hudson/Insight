#include "Resource/Texture.h"
#include "Resource/Texture.inl"

#include "Graphics/RHI/RHI_Texture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Insight
{
	namespace Runtime
	{
		IS_SERIALISABLE_CPP(Texture)

		u32 Texture::GetWidth() const
		{
			return m_width;
		}

		u32 Texture::GetHeight() const
		{
			return m_height;
		}

		u32 Texture::GetDepth() const
		{
			return m_depth;
		}

		PixelFormat Texture::GetFormat() const
		{
			return m_format;
		}

		Graphics::RHI_Texture* Texture::GetRHITexture() const
		{
			return m_rhi_texture;
		}

		std::vector<Byte> Texture::PNG()
		{
			std::vector<Byte> compressTextureData;
			stbi_write_png_to_func([](void* context, void* data, int size)
				{
				std::vector<Byte>* textureData = reinterpret_cast<std::vector<Byte>*>(context);
				textureData->resize(size);
				Platform::MemCopy((void*)textureData->data(), data, size);
				}, &compressTextureData, GetWidth(), GetHeight(), 4, m_rawDataPtr, 0);
			return compressTextureData;
		}
	}
}