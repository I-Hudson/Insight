#pragma once

#include "Graphics/RHI/RHI_PhysicalDevice.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <dxgidebug.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
			// it has no understanding of the lifetime of resources on the GPU. Apps must account
			// for the GPU lifetime of resources to avoid destroying objects that may still be
			// referenced by the GPU.
			// An example of this can be found in the class method: OnDestroy().
			using Microsoft::WRL::ComPtr;

			class RHI_PhysicalDevice_DX12 : public RHI_PhysicalDevice
			{
			public:
				RHI_PhysicalDevice_DX12();
				RHI_PhysicalDevice_DX12(const RHI_PhysicalDevice_DX12& other);
				RHI_PhysicalDevice_DX12(IDXGIAdapter1* device);

				RHI_PhysicalDevice_DX12& operator=(const RHI_PhysicalDevice_DX12& other);
				RHI_PhysicalDevice_DX12& operator=(RHI_PhysicalDevice_DX12&& other);

				ComPtr<IDXGIAdapter1>& GetPhysicalDevice() { return Gpu; }
				DXGI_ADAPTER_DESC1 GetPhysicalDeviceProperties() const { return GpuProps; }

				virtual bool IsValid() const override;
				virtual int GetVendorId() const override;
				virtual std::string GetName() const override;

			private:
				ComPtr<IDXGIAdapter1> Gpu;
				DXGI_ADAPTER_DESC1 GpuProps;
			};
		}
	}
}