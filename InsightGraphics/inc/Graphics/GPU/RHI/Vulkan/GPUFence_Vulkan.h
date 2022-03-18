#pragma once

#include "Graphics/GPU/GPUFence.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUFence_Vulkan : public GPUFence, public GPUResource_Vulkan
			{
			public:
				GPUFence_Vulkan();
				virtual ~GPUFence_Vulkan() override;

				virtual void Wait() override;
				virtual void Reset() override;
				virtual bool IsSignaled() override;

				vk::Fence GetFence() const { return m_fence; }

			protected:
				virtual void Create() override;
				virtual void Destroy() override;

			private:
				vk::Fence m_fence;
			};
		}
	}
}