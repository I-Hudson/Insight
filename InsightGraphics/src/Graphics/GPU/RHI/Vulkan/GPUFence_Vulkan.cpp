#include "Graphics/GPU/RHI/Vulkan/GPUFence_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUFence_Vulkan::GPUFence_Vulkan()
			{
			}

			GPUFence_Vulkan::~GPUFence_Vulkan()
			{
				Destroy();
			}

			void GPUFence_Vulkan::Wait()
			{
				GetDevice()->GetDevice().waitForFences({ m_fence }, true, UINT64_MAX);
			}

			void GPUFence_Vulkan::Reset()
			{
				GetDevice()->GetDevice().resetFences({ m_fence });
			}

			bool GPUFence_Vulkan::IsSignaled()
			{
				return GetDevice()->GetDevice().getFenceStatus(m_fence) == vk::Result::eSuccess;
			}

			void GPUFence_Vulkan::Create()
			{
				vk::FenceCreateInfo info = vk::FenceCreateInfo();
				m_fence = GetDevice()->GetDevice().createFence(info);
			}

			void GPUFence_Vulkan::Destroy()
			{
				if (m_fence)
				{
					GetDevice()->GetDevice().destroyFence(m_fence);
					m_fence = nullptr;
				}
			}
		}
	}
}