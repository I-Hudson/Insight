#include "Graphics/GPU/GPUDevice.h"
#include "Core/Memory.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"
#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		GPUDevice* GPUDevice::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUDevice_Vulkan(); }
			else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUDevice_DX12(); }
			return nullptr;
		}
	}
}