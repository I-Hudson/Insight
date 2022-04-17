#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/RHI/DX12/GPUAdapter_DX12.h"
#include "Graphics/GraphicsManager.h"

#include <map>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class GPUDevice_DX12 : public GPUDevice
			{
			public:
				virtual ~GPUDevice_DX12() { }

				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void WaitForGPU() const override;

				virtual GPUAdapter_DX12* GetAdapter() override;

				IDXGIFactory4* GetFactory() const { return m_factory.Get(); }
				ID3D12Device* GetDevice() const { return m_device.Get(); }
				ID3D12CommandQueue* GetQueue(GPUQueue queue) const;

			private:
				void FindAdapter(IDXGIFactory4* factory, IDXGIAdapter1** ppAdapter);

			private:
				GPUAdapter_DX12 m_adapter;
				ComPtr<IDXGIFactory4> m_factory{ nullptr };
				ComPtr<ID3D12Device> m_device{ nullptr };
				ComPtr<ID3D12Debug> m_debugController{ nullptr };
				std::map<GPUQueue, ComPtr<ID3D12CommandQueue>> m_queues;
			};

			class GPUResource_DX12 : public GPUDeviceResource
			{
			public:
				GPUResource_DX12()
				{
					m_device = dynamic_cast<GPUDevice_DX12*>(GraphicsManager::Instance().m_sharedData.GPUDevice);
				}

				GPUDevice_DX12* GetDevice() { return m_device; }

			private:
				GPUDevice_DX12* m_device;
			};
		}
	}
}