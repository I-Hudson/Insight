#pragma once

#include "Resource/Resource.h"
#include "Resource/Texture2D.h"
#include "Resource/ResourceLifeTimeHandle.h"

//#include "Generated/Material_reflect_generated.h"

#include <array>

namespace Insight
{
	namespace Runtime
	{
		class Texture2D;
		class ModelLoader;

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
		public:
			Material();
			Material(std::string_view filePath);
			virtual ~Material() override;

			REGISTER_RESOURCE(Material);
			IS_SERIALISABLE_H(Material)

			std::string_view GetName() const;

			void SetTexture(TextureTypes texture_type, Texture2D* texture);
			Texture2D* GetTexture(TextureTypes texture_type) const;

			void SetProperty(MaterialProperty property, float value);
			float GetProperty(MaterialProperty property) const;
			std::array<float, static_cast<u32>(MaterialProperty::Count)> GetProperties() const;

			bool operator==(const Material& other) const;
			bool operator!=(const Material& other) const;

		private:
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad() override final;

			/// @brief Save resrouce to disk.
			virtual void Save(const std::string& file_path);

		private:
			std::string m_name;
			std::array<ResourceLFHandle<Texture2D>, static_cast<u32>(TextureTypes::Count)> m_textures;
			std::array<float, static_cast<u32>(MaterialProperty::Count)> m_properties;

			friend class ModelLoader;
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