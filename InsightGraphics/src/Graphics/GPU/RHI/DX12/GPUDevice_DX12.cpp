#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"
#include "Graphics/GPU/RHI/DX12/GPUSwapchain_DX12.h"
#include "Graphics/GPU/RHI/DX12/DX12Utils.h"
#include "Graphics/GPU/RHI/DX12/GPUFence_DX12.h"
#include "Graphics/Window.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			bool GPUDevice_DX12::Init()
			{
				UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
				// Enable the debug layer (requires the Graphics Tools "optional feature").
				// NOTE: Enabling the debug layer after device creation will invalidate the active device.
				{
					if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
					{
						m_debugController->EnableDebugLayer();

						// Enable additional debug layers.
						dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
						
					}
				}
#endif

				ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
				if (!m_factory)
				{
					return false;
				}

				FindAdapter(m_factory.Get(), &m_adapter.GetPhysicalDevice());

				ThrowIfFailed(D3D12CreateDevice(
					m_adapter.GetPhysicalDevice().Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&m_device)
				));

				// Describe and create the command queue.
				D3D12_COMMAND_QUEUE_DESC queueDesc = {};
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

				HRESULT createCommandQueueResult = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GPUQueue_Graphics]));
				if (createCommandQueueResult != S_OK)
				{
					IS_CORE_WARN("[GPUDevice_DX12::Init] Queue not supproted: {}, HR: {}",
						(int)D3D12_COMMAND_LIST_TYPE_DIRECT, HrToString(createCommandQueueResult));
				}

				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
				createCommandQueueResult = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GPUQueue_Compute]));
				if (createCommandQueueResult != S_OK)
				{
					IS_CORE_WARN("[GPUDevice_DX12::Init] Queue not supproted: {}, HR: {}",
						(int)D3D12_COMMAND_LIST_TYPE_COMPUTE, HrToString(createCommandQueueResult));
				}

				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
				createCommandQueueResult = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GPUQueue_Transfer]));
				if (createCommandQueueResult != S_OK)
				{
					IS_CORE_WARN("[GPUDevice_DX12::Init] Queue not supproted: {}, HR: {}",
						(int)D3D12_COMMAND_LIST_TYPE_COPY, HrToString(createCommandQueueResult));
				}

				m_swapchain = new RHI::DX12::GPUSwapchain_DX12();
				m_swapchain->Prepare();
				m_swapchain->Build(GPUSwapchainDesc(Window::Instance().GetWidth(), Window::Instance().GetHeight()));

				return true;
			}

			void GPUDevice_DX12::Destroy()
			{
				m_queues.clear();

				if (m_swapchain)
				{
					m_swapchain->Destroy();
					delete m_swapchain;
					m_swapchain = nullptr;
				}

				if (m_device)
				{
					m_device.Reset();
					m_device = nullptr;
				}

				if (m_factory)
				{
					m_factory.Reset();
					m_factory = nullptr;
				}
				m_adapter = {};

				ComPtr<IDXGIDebug1> dxgiDebug;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
				{
					dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL));
				}
			}

			void GPUDevice_DX12::WaitForGPU() const
			{
				for (const auto& itr : GPUFenceManager::Instance().m_inUseFences)
				{
					itr->Wait();
				}
			}

			GPUAdapter_DX12* GPUDevice_DX12::GetAdapter()
			{
				return &m_adapter;
			}

			ID3D12CommandQueue* GPUDevice_DX12::GetQueue(GPUQueue queue) const
			{
				switch (queue)
				{
				case Insight::Graphics::GPUQueue_Graphics:
				case Insight::Graphics::GPUQueue_Compute:
				case Insight::Graphics::GPUQueue_Transfer:
					return m_queues.at(queue).Get();
				default: break;
				}
				return m_queues.at(GPUQueue_Graphics).Get();
			}

			void GPUDevice_DX12::FindAdapter(IDXGIFactory4* factory, IDXGIAdapter1** ppAdapter)
			{
				*ppAdapter = nullptr;

				ComPtr<IDXGIAdapter1> adapter;
				ComPtr<IDXGIFactory6> factory6;
				if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6))))
				{
					for (
						UINT adapterIndex = 0;
						SUCCEEDED(factory6->EnumAdapterByGpuPreference(
							adapterIndex,
							DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
							IID_PPV_ARGS(&adapter)));
						++adapterIndex)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
						{
							// Don't select the Basic Render Driver adapter.
							// If you want a software adapter, pass in "/warp" on the command line.
							continue;
						}

						// Check to see whether the adapter supports Direct3D 12, but don't create the
						// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
						{
							break;
						}
					}
				}

				if (adapter == nullptr)
				{
					for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
						{
							// Don't select the Basic Render Driver adapter.
							// If you want a software adapter, pass in "/warp" on the command line.
							continue;
						}

						// Check to see whether the adapter supports Direct3D 12, but don't create the
						// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
						{
							break;
						}
					}
				}
				*ppAdapter = adapter.Detach();
			}
		}
	}
}