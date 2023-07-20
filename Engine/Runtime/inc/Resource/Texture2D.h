#pragma once

#include "Resource/Texture.h"

#include "Generated/Texture2D_reflect_generated.h"

#include <Reflect.h>

namespace Insight
{
	namespace Runtime
	{
		class TextureLoader;

		REFLECT_CLASS()
		class IS_RUNTIME Texture2D : public Texture
		{
			REFLECT_GENERATED_BODY()
		public:
			Texture2D(std::string_view filePath);
			virtual ~Texture2D() override;

			REGISTER_RESOURCE(Texture2D);

			virtual void UpdateRHITexture(void* textureData, u64 textureSize) override;

			/// @brief Load all the pixels from disk and return the data.
			/// @return std::vector<Byte>
			std::vector<Byte> GetPixels() const;

		private:
			// Begin -- IResource--
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();
			// End -- IResource--

			/// @brief Save resrouce to disk.
			virtual void Save(const std::string& file_path);

		private:
			void OnRHITextureUploadCompleted(Graphics::RHI_Texture* rhiTexture);

			friend TextureLoader;
		};
	}
}