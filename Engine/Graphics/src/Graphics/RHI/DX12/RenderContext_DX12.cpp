#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/Window.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Event/EventSystem.h"

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
			void FrameSubmitContext_DX12::OnCompleted()
			{
				for (const RHI_CommandList* cmdList : CommandLists)
				{
					if (cmdList)
					{
						cmdList->OnWorkCompleted();
					}
				}

				DescriptorHeapGPURes.Reset();
				DescriptorHeapSampler.Reset();
			}

			RenderContext_DX12::~RenderContext_DX12()
			{
			}

			bool RenderContext_DX12::Init()
			{
				UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
				/// Enable the debug layer (requires the Graphics Tools "optional feature").
				/// NOTE: Enabling the debug layer after device creation will invalidate the active device.
				{
					if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
					{
						m_debugController->EnableDebugLayer();

						/// Enable additional debug layers.
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

				/// Describe and create the command queue.
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

				m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).Create(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).Create(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

				m_swapchainImages.resize(RenderGraph::s_MaxFarmeCount);
				for (size_t i = 0; i < RenderGraph::s_MaxFarmeCount; ++i)
				{
					m_swapchainImages[i].ColourHandle = m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).GetNewHandle();
				}

				CreateSwapchain(Window::Instance().GetWidth(), Window::Instance().GetHeight());

				m_pipelineLayoutManager.SetRenderContext(this);
				m_pipelineManager.SetRenderContext(this);

				m_commandListManager.ForEach([this](CommandListManager& manager)
					{
						manager.Create(this);
					});

				m_submitFrameContexts.Setup();
				m_submitFrameContexts.ForEach([this](FrameSubmitContext_DX12& context)
					{
						ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context.SubmitFence)));
				context.SubmitFenceValue = 0;
				context.SubmitFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (context.SubmitFence == nullptr)
				{
					ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
				}

				context.DescriptorHeapGPURes.SetRenderContext(this);
				context.DescriptorHeapGPURes.Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100000);
				context.DescriptorHeapSampler.SetRenderContext(this);
				context.DescriptorHeapSampler.Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);
					});

				InitImGui();

				m_uploadQueue.Init();

				WaitForGpu();

				return true;
			}

			void RenderContext_DX12::Destroy()
			{
				WaitForGpu();

				DestroyImGui();

				BaseDestroy();

				m_pipelineManager.Destroy();
				m_pipelineLayoutManager.Destroy();

				m_submitFrameContexts.ForEach([](FrameSubmitContext_DX12& context)
					{
						CloseHandle(context.SubmitFenceEvent);
				context.SubmitFence.Reset();
				context.SubmitFence = nullptr;
				context.DescriptorHeapGPURes.Destroy();
				context.DescriptorHeapSampler.Destroy();
					});

				m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).Destroy();


				m_queues.clear();

				for (auto& image : m_swapchainImages)
				{
					Renderer::FreeTexture(image.Colour);
				}

				if (m_swapchain)
				{
					m_swapchainImages.clear();
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
				ImGui_ImplGlfw_InitForOther(Window::Instance().GetRawWindow(), false);

				DescriptorHeapHandle_DX12 handle = m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).GetNewHandle();

				ImGui_ImplDX12_Init(m_device.Get(),
					RenderGraph::s_MaxFarmeCount,
					DXGI_FORMAT_R8G8B8A8_UNORM,
					m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).GetHeap(handle.HeapId),
					handle.CPUPtr,
					handle.GPUPtr);

				ImGui_ImplDX12_NewFrame();
				ImGuiBeginFrame();
			}

			void RenderContext_DX12::DestroyImGui()
			{
				ImGui_ImplDX12_Shutdown();
			}

			bool RenderContext_DX12::PrepareRender()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				{
					IS_PROFILE_SCOPE("ImGui Render");
					ImGuiRender();
				}

				if (Window::Instance().GetWidth() == 0 || Window::Instance().GetHeight() == 0)
				{
					return false;
				}

				if (Window::Instance().GetSize() != m_swapchainBufferSize)
				{
					IS_PROFILE_SCOPE("Swapchain resize");
					SetSwaphchainResolution({ Window::Instance().GetWidth(), Window::Instance().GetHeight() });
					return false;
				}

				{
					IS_PROFILE_SCOPE("Fence wait");
					// First get the status of the fence. Then if it has not finished, wait on it.

					// If the next frame is not ready to be rendered yet, wait until it is ready.
					u64 fenceCompletedValue = m_submitFrameContexts.Get().SubmitFence->GetCompletedValue();
					if (fenceCompletedValue < m_submitFrameContexts.Get().SubmitFenceValue)
					{
						ThrowIfFailed(m_submitFrameContexts.Get().SubmitFence->SetEventOnCompletion(m_submitFrameContexts.Get().SubmitFenceValue, m_submitFrameContexts.Get().SubmitFenceEvent));
						WaitForSingleObjectEx(m_submitFrameContexts.Get().SubmitFenceEvent, INFINITE, FALSE);
					}
					m_submitFrameContexts->OnCompleted();

					m_availableSwapchainImage = m_swapchain->GetCurrentBackBufferIndex();
				}

				m_descriptorSetManager->Reset();
				m_commandListManager->Reset();

				m_resource_tracker.BeginFrame();

				return true;
			}

			void RenderContext_DX12::PreRender(RHI_CommandList* cmdList)
			{
				// Go through out defered manager and call all the functions which have been queued up.
				m_gpu_defered_manager.Update(cmdList);
				m_uploadQueue.UploadToDevice(cmdList);

				RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(cmdList);

#ifdef DX12_ENHANCED_BARRIERS
				// Transition back-buffer to a writable state for rendering.
				CD3DX12_TEXTURE_BARRIER barrier = {};
				barrier.SyncBefore = D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE;
				barrier.SyncAfter = D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_ALL;
				barrier.AccessBefore = D3D12_BARRIER_ACCESS::D3D12_BARRIER_ACCESS_NO_ACCESS;
				barrier.AccessAfter = D3D12_BARRIER_ACCESS::D3D12_BARRIER_ACCESS_RENDER_TARGET;
				barrier.LayoutBefore = D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_PRESENT;
				barrier.LayoutAfter = D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_RENDER_TARGET;
				barrier.pResource = m_swapchainImages[m_availableSwapchainImage].Colour->GetResource();
				cmdListDX12->PipelineBarrierImage({ barrier });
#else
				cmdListDX12->PipelineResourceBarriers({ CD3DX12_RESOURCE_BARRIER::Transition(
					m_swapchainImages[m_availableSwapchainImage].Colour->GetResource(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_RENDER_TARGET) });
#endif
				m_swapchainImages[m_availableSwapchainImage].Colour->SetLayout(ImageLayout::ColourAttachment);
			}

			void RenderContext_DX12::PostRender(RHI_CommandList* cmdList)
			{
				m_submitFrameContexts.Get().CommandLists.clear();
				if (cmdList != nullptr)
				{
					RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(cmdList);

					if (!cmdListDX12->IsDiscard())
					{
						m_submitFrameContexts.Get().CommandLists.push_back(cmdList);

						{
							IS_PROFILE_SCOPE("ExecuteCommandLists");
							ID3D12CommandList* ppCommandLists[] = { cmdListDX12->GetCommandList() };
							m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
						}

						{
							IS_PROFILE_SCOPE("Present");
							// Present the frame.
							if (HRESULT hr = m_swapchain->Present(0, 0); FAILED(hr))
							{
								if (hr == 0x0)
								{
									m_device->GetDeviceRemovedReason();
								}
							}

							// Schedule a Signal command in the queue.
							++m_submitFrameContexts.Get().SubmitFenceValue;
							const UINT64 currentFenceValue = m_submitFrameContexts.Get().SubmitFenceValue;
							ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_submitFrameContexts.Get().SubmitFence.Get(), currentFenceValue));

							m_currentFrame = (m_currentFrame + 1) % RenderGraph::s_MaxFarmeCount;
						}
					}
				}

				{
					IS_PROFILE_SCOPE("ImGui NewFrame");
					ImGui_ImplDX12_NewFrame();
					ImGuiBeginFrame();
				}

				m_resource_tracker.EndFrame();
			}

			//void RenderContext_DX12::Render(RHI_CommandList* cmdList)
			//{
			//	IS_PROFILE_FUNCTION();
			//	ImGuiRender();
			//
			//	FrameResource_DX12& frame = m_frames[m_frameIndex];
			//
			//	frame.Reset();
			//
			//	/// Record cmd buffers and execute
			//	RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(frame.CommandListManager.GetCommandList());
			//
			//	/// Set back buffer texture/image to render target so we can render to it.
			//	cmdListDX12->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainImages[m_frameIndex].Colour.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
			//
			//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapchainImages[m_frameIndex].ColourHandle.GetCPUHandle();
			//	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = m_swapchainImages[m_frameIndex].DepthStencilHandle.GetCPUHandle();
			//
			//	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			//	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
			//
			//	cmdListDX12->ClearRenderTargetView(rtvHandle, &clear_color_with_alpha[0], 0, NULL);
			//	cmdListDX12->ClearDepthStencilView(depthStencilHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			//
			//	cmdListDX12->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthStencilHandle);
			//
			//	cmdListDX12->Record(cmdList, &frame);
			//
			//	{
			//		IS_PROFILE_SCOPE("ImGui_DescriptorHeap");
			//		std::array<ID3D12DescriptorHeap*, 1> imguiHeap = { m_srcImGuiHeap.Get() };
			//		IMGUI_VALID(cmdListDX12->SetDescriptorHeaps(1, imguiHeap.data()));
			//		IMGUI_VALID(ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdListDX12->GetCommandList()));
			//	}
			//
			//	{
			//		IS_PROFILE_SCOPE("ResourceBarrier_swapchain_present");
			//		/// Set back buffer texture/image back to present so we can use it within the swapchain.
			//		cmdListDX12->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapchainImages[m_frameIndex].Colour.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
			//	}
			//	cmdListDX12->Close();
			//
			//	{
			//		IS_PROFILE_SCOPE("ExecuteCommandLists");
			//		ID3D12CommandList* ppCommandLists[] = { cmdListDX12->GetCommandList() };
			//		m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
			//	}
			//
			//	{
			//		IS_PROFILE_SCOPE("Present");
			//		///ZoneScopedN("Present");
			//		/// Present the frame.
			//		if (HRESULT hr = m_swapchain->Present(0, 0); FAILED(hr))
			//		{
			//			if (hr == 0x0)
			//			{
			//				m_device->GetDeviceRemovedReason();
			//			}
			//		}
			//	}
			//
			//	WaitForNextFrame();
			//
			//	if (Window::Instance().GetSize() != m_swapchainSize)
			//	{
			//		IS_PROFILE_SCOPE("Swapchain resize");
			//		WaitForGpu();
			//		m_swapchainSize = Window::Instance().GetSize();
			//		ResizeSwapchainBuffers();
			//	}
			//
			//	{
			//		IS_PROFILE_SCOPE("Imgui End frame");
			//		IMGUI_VALID(ImGui::EndFrame());
			//		IMGUI_VALID(ImGui_ImplDX12_NewFrame());
			//		IMGUI_VALID(ImGuiBeginFrame());
			//	}
			//}

			void RenderContext_DX12::SetSwaphchainResolution(glm::ivec2 resolution)
			{
				WaitForGpu();
				CreateSwapchain(resolution.x, resolution.y);
				Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
			}

			glm::ivec2 RenderContext_DX12::GetSwaphchainResolution() const
			{
				return m_swapchainBufferSize;
			}

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

			void RenderContext_DX12::SetObjectName(std::string_view name, ID3D12Object* handle)
			{
				if (handle)
				{
					std::wstring wString = Platform::WStringFromStringView(name);
					handle->SetName(wString.c_str());
				}
			}

			RHI_Texture* RenderContext_DX12::GetSwaphchainIamge() const
			{
				return m_swapchainImages[m_swapchain->GetCurrentBackBufferIndex()].Colour;
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
							/// Don't select the Basic Render Driver adapter.
							/// If you want a software adapter, pass in "/warp" on the command line.
							continue;
						}

						/// Check to see whether the adapter supports Direct3D 12, but don't create the
						/// actual device yet.
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
							/// Don't select the Basic Render Driver adapter.
							/// If you want a software adapter, pass in "/warp" on the command line.
							continue;
						}

						/// Check to see whether the adapter supports Direct3D 12, but don't create the
						/// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
						{
							break;
						}
					}
				}

				if (adapter)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);

					m_physical_device_info.Device_Name = Platform::StringFromWString(desc.Description);
					m_physical_device_info.Vendor_Id = desc.VendorId;
					m_physical_device_info.VRam_Size += desc.DedicatedVideoMemory;
					m_physical_device_info.SetVendorName();

					m_physical_device_info.MinUniformBufferAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
				}

				*ppAdapter = adapter.Detach();
			}

			void  RenderContext_DX12::CreateSwapchain(u32 width, u32 height)
			{
				if (m_swapchain)
				{
					for (auto& image : m_swapchainImages)
					{
						image.Colour->m_resource.Reset();
						image.Colour->m_resource = nullptr;
					}
					m_swapchain.Reset();
					m_swapchain = nullptr;
				}

				IDXGIFactory4* factory = m_factory.Get();
				ID3D12Device* device = m_device.Get();

				m_swapchainBufferSize = { width, height };

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = RenderGraph::s_MaxFarmeCount;
				swapChainDesc.Width = static_cast<UINT>(m_swapchainBufferSize.x);
				swapChainDesc.Height = static_cast<UINT>(m_swapchainBufferSize.y);
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

				//m_swapchainImages.resize(RenderGraph::s_MaxFarmeCount);
				for (u32 i = 0; i < RenderGraph::s_MaxFarmeCount; ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];

					if (!swapchainImage.Colour)
					{
						RHI_Texture* tex = Renderer::CreateTexture();
						tex->SetName("Swapchain_Image: " + std::to_string(i));
						RHI_Texture_DX12* textureDX12 = static_cast<RHI_Texture_DX12*>(tex);
						swapchainImage.Colour = textureDX12;
						textureDX12->m_allLayerDescriptorHandle = swapchainImage.ColourHandle;
						textureDX12->m_singleLayerDescriptorHandle.push_back(swapchainImage.ColourHandle);

						RHI_TextureInfo textureInfo = {};
						textureInfo.TextureType = TextureType::Tex2D;
						textureInfo.Width = swapChainDesc.Width;
						textureInfo.Height = swapChainDesc.Height;
						textureInfo.Depth = 1;
						textureInfo.Format = PixelFormat::R8G8B8A8_UNorm;
						textureInfo.ImageUsage = 0;
						textureInfo.Layout = ImageLayout::PresentSrc;
						textureInfo.InitalStatus = DeviceUploadStatus::Completed;
						textureDX12->m_infos.push_back(textureInfo);
					}

					/// Get the back buffer from the swapchain.
					ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainImage.Colour->m_resource)));
					device->CreateRenderTargetView(swapchainImage.Colour->GetResource(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());
				}
			}

			void RenderContext_DX12::ResizeSwapchainBuffers()
			{
				/// Release all our previous render targets from the swapchain.
				for (size_t i = 0; i < m_swapchainImages.size(); ++i)
				{
					m_swapchainImages[i].Colour->m_resource.Reset();
					m_swapchainImages[i].Colour->m_resource = nullptr;
				}

				/// Resize our swap chain buffers.
				m_swapchain->ResizeBuffers(RenderGraph::s_MaxFarmeCount, m_swapchainBufferSize.x, m_swapchainBufferSize.y, DXGI_FORMAT_UNKNOWN, 0);
				const UINT frameIndex = m_swapchain->GetCurrentBackBufferIndex();

				/// Create new render targets for the swapchain.
				for (u32 i = 0; i < RenderGraph::s_MaxFarmeCount; ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];
					ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainImage.Colour->m_resource)));
					m_device->CreateRenderTargetView(swapchainImage.Colour->GetResource(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());
				}
			}

			void RenderContext_DX12::WaitForNextFrame()
			{
				IS_PROFILE_FUNCTION();

				/// Schedule a Signal command in the queue.
				//const UINT64 currentFenceValue = m_swapchainFenceValues[m_frameIndex];
				//ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_swapchainFence.Get(), currentFenceValue));
				//
				//
				///// If the next frame is not ready to be rendered yet, wait until it is ready.
				//if (m_swapchainFence->GetCompletedValue() < currentFenceValue)
				//{
				//	ThrowIfFailed(m_swapchainFence->SetEventOnCompletion(currentFenceValue, m_fenceEvent));
				//	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
				//}
				//
				///// Update the frame index.
				//m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
				///// Set the fence value for the next frame.
				//m_swapchainFenceValues[m_frameIndex] = currentFenceValue + 1;
			}

			DescriptorHeap_DX12& RenderContext_DX12::GetDescriptorHeap(DescriptorHeapTypes descriptorHeapType)
			{
				return m_descriptorHeaps.at(descriptorHeapType);
			}

			DescriptorHeapGPU_DX12& RenderContext_DX12::GetFrameDescriptorHeapGPU()
			{
				return m_submitFrameContexts.Get().DescriptorHeapGPURes;
			}

			DescriptorHeapGPU_DX12& RenderContext_DX12::GetFrameDescriptorHeapGPUSampler()
			{
				return m_submitFrameContexts.Get().DescriptorHeapSampler;
			}

			void RenderContext_DX12::WaitForGpu()
			{
				// Increment the fence value for the current frame.
				++m_submitFrameContexts.Get().SubmitFenceValue;
				u64 currentFenceValue = m_submitFrameContexts.Get().SubmitFence->GetCompletedValue();

				// Schedule a Signal command in the queue.
				ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_submitFrameContexts.Get().SubmitFence.Get(), m_submitFrameContexts.Get().SubmitFenceValue));

				// Wait until the fence has been processed.
				ThrowIfFailed(m_submitFrameContexts.Get().SubmitFence->SetEventOnCompletion(m_submitFrameContexts.Get().SubmitFenceValue, m_submitFrameContexts.Get().SubmitFenceEvent));
				WaitForSingleObjectEx(m_submitFrameContexts.Get().SubmitFenceEvent, INFINITE, FALSE);

				currentFenceValue = m_submitFrameContexts.Get().SubmitFence->GetCompletedValue();
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)