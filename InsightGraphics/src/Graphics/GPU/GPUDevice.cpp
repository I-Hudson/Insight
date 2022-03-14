#include "Graphics/GPU/GPUDevice.h"
#include "Core/Memory.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		GPUDevice* GPUDevice::Create()
		{
			return new RHI::Vulkan::GPUDevice_Vulkan();
		}
	}
}