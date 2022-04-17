#include "Graphics/GPU/RHI/DX12/GPUSwapchain_DX12.h"
#include "Graphics/GPU/RHI/DX12/GPUAdapter_DX12.h"
#include "Graphics/GPU/RHI/DX12/GPUSemaphore_DX12.h"
#include "Graphics/GPU/RHI/DX12/GPUFence_DX12.h"
#include "Graphics/GPU/RHI/DX12/DX12Utils.h"
#include "Graphics/Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			GPUSwapchain_DX12::~GPUSwapchain_DX12()
			{
				Destroy();
			}

			void GPUSwapchain_DX12::Prepare()
			{
			}

			void GPUSwapchain_DX12::Build(GPUSwapchainDesc desc)
			{
				m_desc = desc;
				m_desc.ImageCount = m_desc.ImageCount == 0 ? 2 : m_desc.ImageCount;

				Destroy();

				IDXGIFactory4* factory = GetDevice()->GetFactory();
				ID3D12Device* device = GetDevice()->GetDevice();

				// Describe and create a render target view (RTV) descriptor heap.
				D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
				rtvHeapDesc.NumDescriptors = m_desc.ImageCount;
				rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
				m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = m_desc.ImageCount;
				swapChainDesc.Width = m_desc.Width;
				swapChainDesc.Height = m_desc.Height;
				swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.SampleDesc.Count = 1;

				GLFWwindow* window = Window::Instance().GetRawWindow();
				HWND hwmd = glfwGetWin32Window(window);

				ComPtr<IDXGISwapChain1> swapChain;
				ThrowIfFailed(factory->CreateSwapChainForHwnd(
					GetDevice()->GetQueue(GPUQueue_Graphics),
					hwmd,
					&swapChainDesc,
					nullptr,
					nullptr,
					&swapChain));

				swapChain.As(&m_swapChain);

				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
				for (u32 i = 0; i < m_desc.ImageCount; ++i)
				{
					m_swapchainImages.push_back(nullptr);
					ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainImages.back())));
					GetDevice()->GetDevice()->CreateRenderTargetView(m_swapchainImages.back().Get(), nullptr, rtvHandle);
					rtvHandle.ptr += m_rtvDescriptorSize;
				}
			}

			void GPUSwapchain_DX12::Destroy()
			{
				if (m_swapChain)
				{
					for (ComPtr<ID3D12Resource>& image : m_swapchainImages)
					{
						image.Reset();
						image = nullptr;
					}
					m_swapchainImages.clear();
					m_swapChain.Reset();
					m_swapChain = nullptr;
				}
			}

			void GPUSwapchain_DX12::AcquireNextImage(GPUSemaphore* semaphore, GPUFence* fence)
			{
				ID3D12CommandQueue* queue = GetDevice()->GetQueue(GPUQueue_Graphics);

				// Signal to our graphics queue the semaphore and fence.
				if (fence)
				{
					fence->Reset();
				}

				// Update the frame index.
				m_nextImgeIndex = m_swapChain->GetCurrentBackBufferIndex();

				if (semaphore)
				{
					GPUSemaphore_DX12* semaphoreDX12 = dynamic_cast<GPUSemaphore_DX12*>(semaphore);
					//queue->Wait(semaphoreDX12->GetSemaphore(), c_DX12SemaphoreSignal);
				}
				if (fence)
				{
					fence->Wait();
				}
			}

			void GPUSwapchain_DX12::Present(GPUQueue queue, u32 imageIndex, const std::vector<GPUSemaphore*>& semaphores)
			{
				ThrowIfFailed(m_swapChain->Present(1, 0));
			}
		}
	}
}