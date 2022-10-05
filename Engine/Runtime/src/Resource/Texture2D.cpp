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
	}
}