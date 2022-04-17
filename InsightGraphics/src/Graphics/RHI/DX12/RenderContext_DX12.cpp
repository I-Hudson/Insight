#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/Window.h"

#include "backends/imgui_impl_glfw.h"

#ifdef IS_PLATFORM_WINDOWS
#include "backends/imgui_impl_dx12.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#endif

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			bool RenderContext_DX12::Init()
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

				FindPhysicalDevice(&m_physicalDevice.GetPhysicalDevice());

				ThrowIfFailed(D3D12CreateDevice(
					m_physicalDevice.GetPhysicalDevice().Get(),
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

				CreateSwapchain();

				m_frameIndex = 0;
				ThrowIfFailed(m_device->CreateFence(m_swapchainFenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_swapchainFence)));
				m_swapchainFenceValues[m_frameIndex]++;

				// Create an event handle to use for frame synchronization.
				m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (m_fenceEvent == nullptr)
				{
					ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
				}

				for (FrameResourceDX12& frame : m_frames)
				{
					frame.Init(this);
				}

				m_pipelineStateObjectManager.SetRenderContext(this);

				InitImGui();

				WaitForGpu();

				return true;
			}

			void RenderContext_DX12::Destroy()
			{
				WaitForGpu();

				DestroyImGui();

				m_pipelineStateObjectManager.Destroy();

				for (FrameResourceDX12& frame : m_frames)
				{
					frame.Destroy();
				}

				m_queues.clear();

				CloseHandle(m_fenceEvent);
				if (m_swapchainFence)
				{
					m_swapchainFence.Reset();
					m_swapchainFence = nullptr;
				}

				if (m_swapchain)
				{
					m_swapchainImages.clear();
					m_rtvHeap.Reset();
					m_swapchain.Reset();
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
				m_physicalDevice = {};

				ComPtr<IDXGIDebug1> dxgiDebug;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
				{
					dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL));
				}
			}

			void RenderContext_DX12::InitImGui()
			{
				ImGui_ImplGlfw_InitForOther(Window::Instance().GetRawWindow(), true);

				D3D12_DESCRIPTOR_HEAP_DESC desc = {};
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				desc.NumDescriptors = 1;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srcImGuiHeap)));

				ImGui_ImplDX12_Init(m_device.Get(), 
					c_FrameCount,
					DXGI_FORMAT_R8G8B8A8_UNORM, 
					m_srcImGuiHeap.Get(), 
					m_srcImGuiHeap->GetCPUDescriptorHandleForHeapStart(),
					m_srcImGuiHeap->GetGPUDescriptorHandleForHeapStart());

				ImGui_ImplDX12_NewFrame();
				ImGuiBeginFrame();
			}

			void RenderContext_DX12::DestroyImGui()
			{
				ImGui_ImplDX12_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();

				m_srcImGuiHeap.Reset();
				m_srcImGuiHeap = nullptr;
			}

			void RenderContext_DX12::Render(CommandList cmdList)
			{
				ImGuiRender();

				FrameResourceDX12& frame = m_frames[m_frameIndex];

				// Record cmd buffers and execute
				frame.CommandAllocator.Update();
				CommandList_DX12& cmdListDX12 = frame.CommandAllocator.GetCommandList();

				// Set back buffer texture/image to render target so we can render to it.
				cmdListDX12.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainImages[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

				CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
				ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
				const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
				cmdListDX12.m_commandList->ClearRenderTargetView(rtvHandle, clear_color_with_alpha, 0, NULL);
				cmdListDX12.m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

				cmdListDX12.Record(cmdList);

				std::array<ID3D12DescriptorHeap*, 1> imguiHeap = { m_srcImGuiHeap.Get() };
				cmdListDX12.m_commandList->SetDescriptorHeaps(1, imguiHeap.data());
				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdListDX12.m_commandList.Get());

				// Set back buffer texture/image back to present so we can use it within the swapchain.
				cmdListDX12.m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainImages[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
				cmdListDX12.Close();

				ID3D12CommandList* ppCommandLists[] = { cmdListDX12.GetCommandBuffer() };
				m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

				// Present the frame.
				ThrowIfFailed(m_swapchain->Present(1, 0));

				WaitForNextFrame();

				if (Window::Instance().GetSize() != m_swapchainSize)
				{
					WaitForGpu();
					m_swapchainSize = Window::Instance().GetSize();
					ResizeSwapchainBuffers();
				}

				//ImGui::EndFrame();
				//ImGui::UpdatePlatformWindows();

				ImGui_ImplDX12_NewFrame();
				ImGuiBeginFrame();
			}

			RHI_Buffer* RenderContext_DX12::CreateVertexBuffer(u64 sizeBytes)
			{
				return m_vertexBuffer.GetObject(0);

			}

			RHI_Buffer* RenderContext_DX12::CreateIndexBuffer(u64 sizeBytes)
			{
				return nullptr;
			}

			void RenderContext_DX12::FreeVertexBuffer(RHI_Buffer* buffer)
			{
			}

			void RenderContext_DX12::FreeIndexBuffer(RHI_Buffer* buffer)
			{
			}

			void RenderContext_DX12::FindPhysicalDevice(IDXGIAdapter1** ppAdapter)
			{
				*ppAdapter = nullptr;

				IDXGIFactory4* factory = m_factory.Get();

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

			void  RenderContext_DX12::CreateSwapchain()
			{
				if (m_swapchain)
				{
					m_swapchainImages.clear();
					m_rtvHeap.Reset();
					m_swapchain.Reset();
					m_swapchain = nullptr;
				}

				IDXGIFactory4* factory = m_factory.Get();
				ID3D12Device* device = m_device.Get();

				// Describe and create a render target view (RTV) descriptor heap.
				D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
				rtvHeapDesc.NumDescriptors = c_FrameCount;
				rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
				m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				
				m_swapchainSize = { Window::Instance().GetWidth(), Window::Instance().GetHeight() };

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = c_FrameCount;
				swapChainDesc.Width = m_swapchainSize.x;
				swapChainDesc.Height = m_swapchainSize.y;
				swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.SampleDesc.Count = 1;

				GLFWwindow* window = Window::Instance().GetRawWindow();
				HWND hwmd = glfwGetWin32Window(window);

				ComPtr<IDXGISwapChain1> swapchain;
				ThrowIfFailed(factory->CreateSwapChainForHwnd(
					m_queues[GPUQueue_Graphics].Get(),
					hwmd,
					&swapChainDesc,
					nullptr,
					nullptr,
					&swapchain));

				swapchain.As(&m_swapchain);

				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
				for (u32 i = 0; i < c_FrameCount; ++i)
				{
					m_swapchainImages.push_back(nullptr);
					ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainImages.back())));
					device->CreateRenderTargetView(m_swapchainImages.back().Get(), nullptr, rtvHandle);
					rtvHandle.ptr += m_rtvDescriptorSize;
				}
			}

			void RenderContext_DX12::ResizeSwapchainBuffers()
			{
				// Release all our previous render targets from the swapchain.
				for (size_t i = 0; i < m_swapchainImages.size(); ++i)
				{
					m_swapchainImages[i].Reset();
				}

				// Resize our swap chain buffers.
				m_swapchain->ResizeBuffers(c_FrameCount, m_swapchainSize.x, m_swapchainSize.y, DXGI_FORMAT_UNKNOWN, 0);
				m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

				// Create new render targets for teh swapchain.
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
				for (u32 i = 0; i < c_FrameCount; ++i)
				{
					ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainImages[i])));
					m_device->CreateRenderTargetView(m_swapchainImages[i].Get(), nullptr, rtvHandle);
					rtvHandle.ptr += m_rtvDescriptorSize;
				}
			}

			void RenderContext_DX12::WaitForNextFrame()
			{
				// Schedule a Signal command in the queue.
				const UINT64 currentFenceValue = m_swapchainFenceValues[m_frameIndex];
				ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_swapchainFence.Get(), currentFenceValue));

				// Update the frame index.
				m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

				// If the next frame is not ready to be rendered yet, wait until it is ready.
				if (m_swapchainFence->GetCompletedValue() < currentFenceValue)
				{
					ThrowIfFailed(m_swapchainFence->SetEventOnCompletion(currentFenceValue, m_fenceEvent));
					WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
				}

				// Set the fence value for the next frame.
				m_swapchainFenceValues[m_frameIndex] = currentFenceValue + 1;
			}

			void RenderContext_DX12::WaitForGpu()
			{
				// Schedule a Signal command in the queue.
				ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_swapchainFence.Get(), m_swapchainFenceValues[m_frameIndex]));

				// Wait until the fence has been processed.
				ThrowIfFailed(m_swapchainFence->SetEventOnCompletion(m_swapchainFenceValues[m_frameIndex], m_fenceEvent));
				WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

				// Increment the fence value for the current frame.
				m_swapchainFenceValues[m_frameIndex]++;
			}


			void RenderContext_DX12::FrameResourceDX12::Init(RenderContext_DX12* context)
			{
				Context = context;
				CommandAllocator.Init(Context);
			}

			void RenderContext_DX12::FrameResourceDX12::Destroy()
			{
				CommandAllocator.Destroy();
			}
}
	}
}