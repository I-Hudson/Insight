#include "Resource/Texture.h"
#include "Resource/Texture.inl"

#include "Graphics/RHI/RHI_Texture.h"

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
	}
}