#include "Resource/Material.h"

namespace Insight
{
	namespace Runtime
	{
		void Material::SetTexture(TextureTypes texture_type, Texture2D* texture)
		{
			m_textures.at(static_cast<u32>(texture_type)) = texture;
		}

		Texture2D* Material::GetTexture(TextureTypes texture_type) const
		{
			return m_textures.at(static_cast<u32>(texture_type));
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