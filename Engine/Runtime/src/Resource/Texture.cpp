#include "Resource/Texture.h"

namespace Insight
{
	namespace Runtime
	{
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

		void Texture::Load()
		{

		}

		void Texture::UnLoad()
		{

		}
	}
}