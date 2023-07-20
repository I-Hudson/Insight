#pragma once

#include "Resource/Resource.h"
#include "Graphics/PixelFormat.h"

#include "Generated/Texture_reflect_generated.h"

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

		/// @brief Define how a texture's data is stored on disk.
		enum class TextureDiskPackedType
		{
			/// @brief Texture data is stores in its original format. Example:
			/// If the texture was a .png, then the texture data is of type .png format.
			Packed,
			/// @brief Texture data has been loaded from its original format.
			/// This will cause higher memory use as the texture data won't be 
			/// in its compressed form from its original format.
			Unpacked,
		};

		enum class TextureDiskFormat
		{
			/// @brief Any other format. used with stb_iamge.
			Other,
			/// @brief Quite OK Image Format used for engine formatted texture files.
			QOI,
		};

		REFLECT_CLASS()
		class IS_RUNTIME Texture : public IResource
		{
			REFLECT_GENERATED_BODY()
		public:
			Texture(std::string_view filePath);

			REGISTER_RESOURCE(Texture);
			IS_SERIALISABLE_H(Texture);

			virtual void UpdateRHITexture(void* textureData, u64 textureSize) { }

			u32 GetWidth() const;
			u32 GetHeight() const;
			u32 GetDepth() const;
			PixelFormat GetFormat() const;

			Graphics::RHI_Texture* GetRHITexture() const;

		protected:
			virtual ResourceId ConvertToEngineFormat() override;

		protected:
			u32 m_width = 0;
			u32 m_height = 0;
			u32 m_depth = 0;
			PixelFormat m_format;
			TextureDiskPackedType m_diskPackedType = TextureDiskPackedType::Packed;
			TextureDiskFormat m_diskFormat = TextureDiskFormat::Other;

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

	OBJECT_SERIALISER(Runtime::Texture, 2,
		SERIALISE_BASE(Runtime::IResource, 1, 0)
		SERIALISE_PROPERTY(u32, m_width, 1, 0)
		SERIALISE_PROPERTY(u32, m_height, 1, 0)
		SERIALISE_PROPERTY(u32, m_depth, 1, 0)
		SERIALISE_PROPERTY(PixelFormat, m_format, 1, 0)
		SERIALSIE_ENGINE_FORMAT(SERIALISE_COMPLEX(Serialisation::SerialiseTextureData, m_rawDataPtr, 1, 0))
		SERIALISE_PROPERTY(Runtime::TextureDiskPackedType, m_diskPackedType, 2, 0)
		SERIALISE_PROPERTY(Runtime::TextureDiskFormat, m_diskFormat, 2, 0)
	);
}