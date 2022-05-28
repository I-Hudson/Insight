#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			RHI_PhysicalDevice_DX12::RHI_PhysicalDevice_DX12()
				: Gpu(nullptr)
			{
			}

			RHI_PhysicalDevice_DX12::RHI_PhysicalDevice_DX12(const RHI_PhysicalDevice_DX12& other)
			{
				*this = other;
			}

			RHI_PhysicalDevice_DX12::RHI_PhysicalDevice_DX12(IDXGIAdapter1* device)
			{
				Gpu = device;
				Gpu->GetDesc1(&GpuProps);
			}

			RHI_PhysicalDevice_DX12& RHI_PhysicalDevice_DX12::operator=(const RHI_PhysicalDevice_DX12& other)
			{
				Gpu = other.Gpu;
				GpuProps = other.GpuProps;
				return *this;
			}

			RHI_PhysicalDevice_DX12& RHI_PhysicalDevice_DX12::operator=(RHI_PhysicalDevice_DX12&& other)
			{
				Gpu = std::move(other.Gpu);
				GpuProps = std::move(other.GpuProps);
				return *this;
			}

			bool RHI_PhysicalDevice_DX12::IsValid() const
			{
				return Gpu != nullptr;
			}

			int RHI_PhysicalDevice_DX12::GetVendorId() const
			{
				return GpuProps.VendorId;
			}

			std::string RHI_PhysicalDevice_DX12::GetName() const
			{
				return (char*)GpuProps.Description;
			}
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)