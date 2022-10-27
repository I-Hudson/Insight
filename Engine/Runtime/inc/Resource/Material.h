#pragma once

#include "Resource/Resource.h"
#include "Resource/Texture.h"

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

		class Material : public IResource
		{
			REGISTER_RESOURCE(Material);
		public:
			Material();
			virtual ~Material() override;


			void SetTexture(TextureTypes texture_type, Texture2D* texture);
			Texture2D* GetTexture(TextureTypes texture_type) const;

			void SetProperty(MaterialProperty property, float value);
			float GetProperty(MaterialProperty property) const;

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
			std::array<Texture2D*, static_cast<u32>(TextureTypes::Count)> m_textures;
			std::array<float, static_cast<u32>(MaterialProperty::Count)> m_properties;

			friend class AssimpLoader;
		};
	}
}