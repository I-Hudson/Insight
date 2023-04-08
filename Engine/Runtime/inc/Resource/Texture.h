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

		class IS_RUNTIME Texture : public IResource
		{
			REGISTER_RESOURCE(Texture);
			IS_SERIALISABLE_H(Texture);
		public:

			u32 GetWidth() const;
			u32 GetHeight() const;
			u32 GetDepth() const;
			PixelFormat GetFormat() const;

			Graphics::RHI_Texture* GetRHITexture() const;

		private:
			std::vector<Byte> PNG();

		protected:
			u32 m_width = 0;
			u32 m_height = 0;
			u32 m_depth = 0;
			PixelFormat m_format;

			u64 m_dataSize = 0;
			/// @brief Store the source file data. This will include the source file format 
			/// optimisations. Example: If a texture of format .png is loaded this will store the data
			/// of the .png as a .png. So when using this pointer you need to call 'stbi_load_from_memory'
			/// to get usable data.
			Byte* m_rawDataPtr = nullptr;
			Graphics::RHI_Texture* m_rhi_texture = nullptr;
		};
	}

	namespace Serialisation
	{
		struct SerialiseTextureData { };
		template<>
		struct ComplexSerialiser<SerialiseTextureData, Byte*, Runtime::Texture>
		{
			void operator()(ISerialiser* serialiser, Byte*& data, Runtime::Texture* texture) const;
		};
	}

	OBJECT_SERIALISER(Runtime::Texture, 1,
		SERIALISE_BASE(Runtime::IResource, 1, 0)
		SERIALISE_PROPERTY(u32, m_width, 1, 0)
		SERIALISE_PROPERTY(u32, m_height, 1, 0)
		SERIALISE_PROPERTY(u32, m_depth, 1, 0)
		//SERIALISE_PROPERTY(PixelFormat, m_format, 1, 0)
		SERIALSIE_ENGINE_FORMAT(SERIALISE_COMPLEX(Serialisation::SerialiseTextureData, m_rawDataPtr, 1, 0))
	);
}