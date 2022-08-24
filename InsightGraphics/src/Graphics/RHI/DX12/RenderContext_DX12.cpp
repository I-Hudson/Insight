#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/Window.h"

#include "Core/Profiler.h"

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

				m_rtvHeap.SetRenderContext(this);
				m_rtvHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				m_dsvHeap.SetRenderContext(this);
				m_dsvHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
				
				m_swapchainImages.resize(RenderGraph::s_FarmeCount);
				for (size_t i = 0; i < RenderGraph::s_FarmeCount; ++i)
				{
					m_swapchainImages[i].ColourHandle = m_rtvHeap.GetNewHandle();
					m_swapchainImages[i].DepthStencilHandle = m_dsvHeap.GetNewHandle();
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

				for (FrameResource_DX12& frame : m_frames)
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

				m_rtvHeap = { };
				m_dsvHeap = { };

				for (FrameResource_DX12& frame : m_frames)
				{
					frame.Destroy();
				}

				BaseDestroy();

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
				IMGUI_VALID(ImGui_ImplGlfw_InitForOther(Window::Instance().GetRawWindow(), true));

				D3D12_DESCRIPTOR_HEAP_DESC desc = {};
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				desc.NumDescriptors = 1;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				IMGUI_VALID(ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srcImGuiHeap))));

				IMGUI_VALID(ImGui_ImplDX12_Init(m_device.Get(),
					RenderGraph::s_FarmeCount,
					DXGI_FORMAT_R8G8B8A8_UNORM,
					m_srcImGuiHeap.Get(),
					m_srcImGuiHeap->GetCPUDescriptorHandleForHeapStart(),
					m_srcImGuiHeap->GetGPUDescriptorHandleForHeapStart()));

				IMGUI_VALID(ImGui_ImplDX12_NewFrame());
				IMGUI_VALID(ImGuiBeginFrame());
			}

			void RenderContext_DX12::DestroyImGui()
			{
				IMGUI_VALID(ImGui_ImplDX12_Shutdown());
				IMGUI_VALID(ImGui_ImplGlfw_Shutdown());
				IMGUI_VALID(ImGui::DestroyContext());

				m_srcImGuiHeap.Reset();
				m_srcImGuiHeap = nullptr;
			}

			//void RenderContext_DX12::Render(CommandList cmdList)
			//{
			//	IS_PROFILE_FUNCTION();
			//	ImGuiRender();

			//	FrameResource_DX12& frame = m_frames[m_frameIndex];

			//	frame.Reset();

			//	// Record cmd buffers and execute
			//	RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(frame.CommandListManager.GetCommandList());

			//	// Set back buffer texture/image to render target so we can render to it.
			//	cmdListDX12->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainImages[m_frameIndex].Colour.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

			//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapchainImages[m_frameIndex].ColourHandle.GetCPUHandle();
			//	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = m_swapchainImages[m_frameIndex].DepthStencilHandle.GetCPUHandle();

			//	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			//	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

			//	cmdListDX12->ClearRenderTargetView(rtvHandle, &clear_color_with_alpha[0], 0, NULL);
			//	cmdListDX12->ClearDepthStencilView(depthStencilHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			//	cmdListDX12->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthStencilHandle);

			//	cmdListDX12->Record(cmdList, &frame);

			//	{
			//		IS_PROFILE_SCOPE("ImGui_DescriptorHeap");
			//		std::array<ID3D12DescriptorHeap*, 1> imguiHeap = { m_srcImGuiHeap.Get() };
			//		IMGUI_VALID(cmdListDX12->SetDescriptorHeaps(1, imguiHeap.data()));
			//		IMGUI_VALID(ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdListDX12->GetCommandList()));
			//	}

			//	{
			//		IS_PROFILE_SCOPE("ResourceBarrier_swapchain_present");
			//		// Set back buffer texture/image back to present so we can use it within the swapchain.
			//		cmdListDX12->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainImages[m_frameIndex].Colour.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
			//	}
			//	cmdListDX12->Close();

			//	{
			//		IS_PROFILE_SCOPE("ExecuteCommandLists");
			//		ID3D12CommandList* ppCommandLists[] = { cmdListDX12->GetCommandList() };
			//		m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
			//	}

			//	{
			//		IS_PROFILE_SCOPE("Present");
			//		//ZoneScopedN("Present");
			//		// Present the frame.
			//		if (HRESULT hr = m_swapchain->Present(0, 0); FAILED(hr))
			//		{
			//			if (hr == 0x0)
			//			{
			//				m_device->GetDeviceRemovedReason();
			//			}
			//		}
			//	}

			//	WaitForNextFrame();

			//	if (Window::Instance().GetSize() != m_swapchainSize)
			//	{
			//		IS_PROFILE_SCOPE("Swapchain resize");
			//		WaitForGpu();
			//		m_swapchainSize = Window::Instance().GetSize();
			//		ResizeSwapchainBuffers();
			//	}

			//	{
			//		IS_PROFILE_SCOPE("Imgui End frame");
			//		IMGUI_VALID(ImGui::EndFrame());
			//		IMGUI_VALID(ImGui_ImplDX12_NewFrame());
			//		IMGUI_VALID(ImGuiBeginFrame());
			//	}
			//}

			void RenderContext_DX12::GpuWaitForIdle()
			{
				WaitForGpu();
			}

			void RenderContext_DX12::SubmitCommandListAndWait(RHI_CommandList* cmdList)
			{
				const RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(cmdList);
				ID3D12CommandList* ppCommandLists[] = { cmdListDX12->GetCommandList() };
				m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				GpuWaitForIdle();
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
				//D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
				//rtvHeapDesc.NumDescriptors = RenderGraph::s_FarmeCount;
				//rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				//rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				//ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
				//m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				m_swapchainSize = { Window::Instance().GetWidth(), Window::Instance().GetHeight() };

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = RenderGraph::s_FarmeCount;
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

				m_swapchainImages.resize(RenderGraph::s_FarmeCount);
				for (u32 i = 0; i < RenderGraph::s_FarmeCount; ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];
					swapchainImage.Colour.Reset();
					swapchainImage.DepthStencil.Reset();

					// Get the back buffer from the swapchain.
					ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainImage.Colour)));
					device->CreateRenderTargetView(swapchainImage.Colour.Get(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());

					D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
					depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
					depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

					D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
					depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
					depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
					depthOptimizedClearValue.DepthStencil.Stencil = 0;

					device->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, 
							Window::Instance().GetWidth(), 
							Window::Instance().GetHeight(), 
							1, 
							0, 
							1, 
							0, 
							D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
						D3D12_RESOURCE_STATE_DEPTH_WRITE,
						&depthOptimizedClearValue,
						IID_PPV_ARGS(&swapchainImage.DepthStencil)
					);
					swapchainImage.DepthStencil->SetName((std::wstring(L"Depth/Stencil Resource Heap_Frame_") + std::to_wstring(i)).c_str());
					device->CreateDepthStencilView(swapchainImage.DepthStencil.Get(), &depthStencilDesc, swapchainImage.DepthStencilHandle.GetCPUHandle());
				}
			}

			void RenderContext_DX12::ResizeSwapchainBuffers()
			{
				// Release all our previous render targets from the swapchain.
				for (size_t i = 0; i < m_swapchainImages.size(); ++i)
				{
					m_swapchainImages[i].Colour.Reset();
					m_swapchainImages[i].DepthStencil.Reset();
				}

				// Resize our swap chain buffers.
				m_swapchain->ResizeBuffers(RenderGraph::s_FarmeCount, m_swapchainSize.x, m_swapchainSize.y, DXGI_FORMAT_UNKNOWN, 0);
				m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

				// Create new render targets for the swapchain.
				for (u32 i = 0; i < RenderGraph::s_FarmeCount; ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];

					ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_swapchainImages[i].Colour)));
					m_device->CreateRenderTargetView(m_swapchainImages[i].Colour.Get(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());

					D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
					depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
					depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

					D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
					depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
					depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
					depthOptimizedClearValue.DepthStencil.Stencil = 0;

					m_device->CreateCommittedResource(
						&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
						D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
							Window::Instance().GetWidth(),
							Window::Instance().GetHeight(),
							1,
							0,
							1,
							0,
							D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
						D3D12_RESOURCE_STATE_DEPTH_WRITE,
						&depthOptimizedClearValue,
						IID_PPV_ARGS(&swapchainImage.DepthStencil)
					);
					swapchainImage.DepthStencil->SetName((std::wstring(L"Depth/Stencil Resource Heap_Frame_") + std::to_wstring(i)).c_str());
					m_device->CreateDepthStencilView(swapchainImage.DepthStencil.Get(), &depthStencilDesc, swapchainImage.DepthStencilHandle.GetCPUHandle());
				}
			}

			void RenderContext_DX12::WaitForNextFrame()
			{
				IS_PROFILE_FUNCTION();
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


			void FrameResource_DX12::Init(RenderContext_DX12* context)
			{
				Context = context;
				CommandListManager.Create(Context);
				UniformBuffer.Create(Context);
				//DescriptorAllocator.SetRenderContext(Context);
			}

			void FrameResource_DX12::Destroy()
			{
				CommandListManager.Destroy();
				UniformBuffer.Release();
				//DescriptorAllocator.Destroy();
			}

			void FrameResource_DX12::Reset()
			{
				FrameResouce::Reset();
				//DescriptorAllocator.Reset();
			}
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)