#include "Graphics/RHI/RHI_Buffer.h"
#include "Core/Memory.h"

#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_Buffer* RHI_Buffer::New()
		{
			if (GraphicsManager::IsVulkan()) { return nullptr; }
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_Buffer_DX12); }
			return nullptr;
		}
	}
}
