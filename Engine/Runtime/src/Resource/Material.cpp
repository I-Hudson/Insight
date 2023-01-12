#include "Resource/Material.h"

namespace Insight
{
	namespace Runtime
	{
		Material::Material()
			: m_properties({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f })
		{ }

		Material::~Material()
		{ }

		void Material::SetTexture(TextureTypes texture_type, Texture2D* texture)
		{
			m_textures.at(static_cast<u32>(texture_type)) = texture;
		}

		Texture2D* Material::GetTexture(TextureTypes texture_type) const
		{
			return m_textures.at(static_cast<u32>(texture_type));
		}

		void Material::SetProperty(MaterialProperty property, float value)
		{
			m_properties.at(static_cast<u32>(property)) = value;
		}

		float Material::GetProperty(MaterialProperty property) const
		{
			return m_properties.at(static_cast<u32>(property));
		}

		std::array<float, static_cast<u32>(MaterialProperty::Count)> Material::GetProperties() const
		{
			return m_properties;
		}

		bool Material::operator==(const Material& other) const
		{
			for (size_t i = 0; i < static_cast<u64>(TextureTypes::Count); ++i)
			{
				if (m_textures.at(i) != other.m_textures.at(i))
				{
					return false;
				}
			}
			for (size_t i = 0; i < static_cast<u64>(MaterialProperty::Count); ++i)
			{
				if (m_properties.at(i) != other.m_properties.at(i))
				{
					return false;
				}
			}
			return true;
		}

		bool Material::operator!=(const Material& other) const
		{
			return !(*this == other);
		}

		void Material::Load()
		{
		}

		void Material::LoadFromMemory(const void* data, u64 size_in_bytes)
		{
		}

		void Material::UnLoad()
		{
		}

		void Material::Save(const std::string& file_path)
		{
		}
	}
}