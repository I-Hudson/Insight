#include "Graphics/GPU/RHI/DX12/GPUAdapter_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			GPUAdapter_DX12::GPUAdapter_DX12()
				: Gpu(nullptr)
			{
			}

			GPUAdapter_DX12::GPUAdapter_DX12(const GPUAdapter_DX12& other)
			{
				*this = other;
			}

			GPUAdapter_DX12::GPUAdapter_DX12(IDXGIAdapter1* device)
			{
				Gpu = device;
				Gpu->GetDesc1(&GpuProps);
			}

			GPUAdapter_DX12& GPUAdapter_DX12::operator=(const GPUAdapter_DX12& other)
			{
				Gpu = other.Gpu;
				GpuProps = other.GpuProps;
				return *this;
			}

			GPUAdapter_DX12& GPUAdapter_DX12::operator=(GPUAdapter_DX12&& other)
			{
				Gpu = std::move(other.Gpu);
				GpuProps = std::move(other.GpuProps);
				return *this;
			}

			bool GPUAdapter_DX12::IsValid() const
			{
				return Gpu != nullptr;
			}

			int GPUAdapter_DX12::GetVendorId() const
			{
				return GpuProps.VendorId;
			}

			std::string GPUAdapter_DX12::GetName() const
			{
				return (char*)GpuProps.Description;
			}
		}
	}
}