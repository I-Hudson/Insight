#include "Graphics/RenderTarget.h"
#include "Graphics/GPU/GPUImage.h"

namespace Insight
{
	namespace Graphics
	{
		RenderTarget::RenderTarget()
		{
		}

		RenderTarget::~RenderTarget()
		{
			Destroy();
		}

		void RenderTarget::Create(std::string key, RenderTargetDesc desc)
		{
			m_image	= GPUImageManager::Instance().CreateOrGetImage(key);
			//m_image->
		}

		void RenderTarget::Destroy()
		{
			if (m_image)
			{
				m_image = nullptr;
			}
		}
	}
}