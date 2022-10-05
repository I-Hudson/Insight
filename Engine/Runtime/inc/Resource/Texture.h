#pragma once

#include "Resource/Resource.h"
#include "Graphics/PixelFormat.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Texture;
	}

	namespace Runtime
	{
		enum class TextureTypes
		{
			Diffuse,
			Normal,
			Specular,

			Count
		};

		class Texture : public IResource
		{
			REGISTER_RESOURCE(Texture);

		public:

			u32 GetWidth() const;
			u32 GetHeight() const;
			u32 GetDepth() const;
			PixelFormat GetFormat() const;

			Graphics::RHI_Texture* GetRHITexture() const;

		protected:
			u32 m_width = 0;
			u32 m_height = 0;
			u32 m_depth = 0;
			PixelFormat m_format;

			Graphics::RHI_Texture* m_rhi_texture = nullptr;
		};
	}
}