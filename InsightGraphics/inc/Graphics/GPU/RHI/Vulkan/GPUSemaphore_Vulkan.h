#pragma once

#include "Graphics/GPU/GPUSemaphore.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUSemaphore_Vulkan : public GPUResource_Vulkan, public GPUSemaphore
			{
			public:
				virtual ~GPUSemaphore_Vulkan() override { }

				virtual void Signal() override;
				vk::Semaphore GetSemaphore() const { return m_semaphore; }

			protected:
				virtual void Create(bool signaled) override;
				virtual void Destroy() override;

			private:
				vk::Semaphore m_semaphore{nullptr};
			};
		}
	}
}