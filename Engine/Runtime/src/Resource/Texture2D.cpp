#include "Resource/Texture2D.h"
#include "Graphics/RenderContext.h"

#include <stb_image.h>

namespace Insight
{
	namespace Runtime
	{
		void Texture2D::Load()
		{
			ASSERT(!m_rhi_texture);
			m_rhi_texture = Renderer::CreateTexture();

			m_rhi_texture->OnUploadCompleted.Bind<&Texture2D::OnRHITextureUploadCompleted>(this);
			m_rhi_texture->LoadFromFile(m_file_path);

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