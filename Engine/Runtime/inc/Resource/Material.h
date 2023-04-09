#pragma once

#include "Resource/Resource.h"
#include "Resource/Texture2D.h"
#include "Resource/ResourceLifeTimeHandle.h"

#include <array>

namespace Insight
{
	namespace Runtime
	{
		class Texture2D;
		class AssimpLoader;

		enum class MaterialProperty
		{
			Colour_R,
			Colour_G,
			Colour_B,
			Colour_A,

			UV_Tilling_X,
			UV_Tilling_Y,

			Count
		};

		class IS_RUNTIME Material : public IResource
		{
			REGISTER_RESOURCE(Material);
		public:
			Material();
			Material(std::string_view filePath);
			virtual ~Material() override;

			IS_SERIALISABLE_H(Material)

			void SetTexture(TextureTypes texture_type, Texture2D* texture);
			Texture2D* GetTexture(TextureTypes texture_type) const;

			void SetProperty(MaterialProperty property, float value);
			float GetProperty(MaterialProperty property) const;
			std::array<float, static_cast<u32>(MaterialProperty::Count)> GetProperties() const;

			bool operator==(const Material& other) const;
			bool operator!=(const Material& other) const;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load();
			/// @brief Handle loading a resource from memory. This is called for a resource which is a "sub resource" of another one
			/// and exists inside another resource's disk file.
			/// @param file_path 
			virtual void LoadFromMemory(const void* data, u64 size_in_bytes);
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();
			/// @brief Save resrouce to disk.
			virtual void Save(const std::string& file_path);

		private:
			std::array<ResourceLFHandle<Texture2D>, static_cast<u32>(TextureTypes::Count)> m_textures;
			std::array<float, static_cast<u32>(MaterialProperty::Count)> m_properties;

			friend class AssimpLoader;
		};
	}

	namespace Serialisation
	{
		struct MaterialTextureSerialise {};
		template<>
		struct ComplexSerialiser<MaterialTextureSerialise, std::array<Runtime::ResourceLFHandle<Runtime::Texture2D>, static_cast<u32>(Runtime::TextureTypes::Count)>, Runtime::Material>
		{
			void operator()(ISerialiser* serialiser
				, std::array<Runtime::ResourceLFHandle<Runtime::Texture2D>, static_cast<u32>(Runtime::TextureTypes::Count)>& textures
				, Runtime::Material* material) const;
		};
	}

	OBJECT_SERIALISER(Runtime::Material, 1,
		SERIALISE_BASE(Runtime::IResource, 1, 0)
		SERIALISE_COMPLEX(Serialisation::MaterialTextureSerialise, m_textures, 1, 0)
		SERIALISE_ARRAY_PROPERTY(float, m_properties, 1, 0)
	);
}