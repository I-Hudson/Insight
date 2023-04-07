#include "Resource/Texture2D.h"
#include "Graphics/RenderContext.h"

#include "FileSystem/FileSystem.h"
#include "Core/Profiler.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace Insight
{
	namespace Runtime
	{
		void Texture2D::Load()
		{
			ASSERT(!m_rhi_texture && !m_rawDataPtr);
			m_rhi_texture = Renderer::CreateTexture();

			m_rhi_texture->OnUploadCompleted.Bind<&Texture2D::OnRHITextureUploadCompleted>(this);

			std::string_view filePath = m_file_path;
			if (filePath.empty() || !FileSystem::FileSystem::Exists(filePath))
			{
				m_resource_state = EResoruceStates::Failed_To_Load;
				return;
			}

			int width, height, channels;
			Byte* data = nullptr;
			{
				IS_PROFILE_SCOPE("stbi_load");
				data = stbi_load(filePath.data(), &width, &height, &channels, STBI_rgb_alpha);
				channels = STBI_rgb_alpha;
			}
			if (!data)
			{
				m_resource_state = EResoruceStates::Failed_To_Load;
				return;
			}

			m_width = width;
			m_height = height;
			m_depth = 1;

			m_dataSize = width * height * STBI_rgb_alpha;
			m_rawDataPtr = static_cast<Byte*>(NewBytes(m_dataSize, Core::MemoryAllocCategory::Resources));
			Platform::MemCopy(m_rawDataPtr, data, m_dataSize);

			stbi_image_free(data);
			
			m_rhi_texture->LoadFromData(m_rawDataPtr, GetWidth(), GetHeight(), GetDepth(), STBI_rgb_alpha);

			m_resource_state = EResoruceStates::Loaded;
		}

		void Texture2D::LoadFromMemory(const void* data, u64 size_in_bytes)
		{
			FAIL_ASSERT();
		}

		void Texture2D::UnLoad()
		{
			Renderer::FreeTexture(m_rhi_texture);
			m_rhi_texture = nullptr;
			
			DeleteBytes(m_rawDataPtr);

			m_resource_state = EResoruceStates::Unloaded;
		}

		void Texture2D::Save(const std::string& file_path)
		{
			FAIL_ASSERT();
		}

		void Texture2D::OnRHITextureUploadCompleted(Graphics::RHI_Texture* rhiTexture)
		{
			ASSERT(rhiTexture == m_rhi_texture);

			rhiTexture->OnUploadCompleted.Unbind<&Texture2D::OnRHITextureUploadCompleted>(this);
			m_width = rhiTexture->GetWidth(0);
			m_height = rhiTexture->GetHeight(0);
			m_depth = rhiTexture->GetInfo(0).Depth;
			m_format = rhiTexture->GetFormat();
		}
	}
}