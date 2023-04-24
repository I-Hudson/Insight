#if defined(IS_DX12_ENABLED)

#ifdef IS_PLATFORM_WINDOWS

#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/Window.h"

#include "Graphics/RHI/RHI_GPUCrashTracker.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "Event/EventSystem.h"

#include "backends/imgui_impl_glfw.h"


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
			static void* D3D12Allocate(size_t Size, size_t Alignment, void* pPrivateData)
			{
				return NewBytes(Size, Core::MemoryAllocCategory::Graphics);
			}
			static void D3D12Free(void* pMemory, void* pPrivateData)
			{
				DeleteBytes(pMemory);
			}

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

			bool RenderContext_DX12::Init(RenderContextDesc desc)
			{
				m_desc = desc;
				UINT dxgiFactoryFlags = 0;

				if (m_desc.GPUValidation)
				{
					//#if defined(_DEBUG)
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
					//#endif
				}

				ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
				if (!m_factory)
				{
					return false;
				}

				FindPhysicalDevice(&m_physicalDevice.GetPhysicalDevice());

				m_gpuCrashTracker = RHI_GPUCrashTracker::Create();
				if (m_gpuCrashTracker)
				{
					m_gpuCrashTracker->Init();
				}

				ThrowIfFailed(D3D12CreateDevice(
					m_physicalDevice.GetPhysicalDevice().Get(),
					m_d3dFeatureLevel,
					IID_PPV_ARGS(&m_device)
				));

				m_d3d12maAllocationCallbacks.pAllocate = D3D12Allocate;
				m_d3d12maAllocationCallbacks.pFree = D3D12Free;

				D3D12MA::ALLOCATOR_DESC d3d12MA_AllocatorDesc = {};
				d3d12MA_AllocatorDesc.pDevice = m_device.Get();
				d3d12MA_AllocatorDesc.pAdapter = m_physicalDevice.GetPhysicalDevice().Get();
				d3d12MA_AllocatorDesc.pAllocationCallbacks = &m_d3d12maAllocationCallbacks;
				ThrowIfFailed(D3D12MA::CreateAllocator(&d3d12MA_AllocatorDesc, &m_d3d12MA));

				/// Describe and create the command queue.
				D3D12_COMMAND_QUEUE_DESC queueDesc = {};
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

				m_graphicsQueue.Initialise(m_device.Get(), GPUQueue_Graphics);

				HRESULT createCommandQueueResult;
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
				m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).Create(DescriptorHeapTypes::CBV_SRV_UAV);

				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).Create(DescriptorHeapTypes::Sampler);

				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).Create(DescriptorHeapTypes::RenderTargetView);

				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).SetRenderContext(this);
				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).Create(DescriptorHeapTypes::DepthStencilView);


				// Create all our null handles for descriptors.
				m_cbvNullHandle = m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).GetNewHandle();
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
				m_device->CreateConstantBufferView(&cbv_desc, m_cbvNullHandle.CPUPtr);

				m_srvNullHandle = m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).GetNewHandle();
				D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
				srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				m_device->CreateShaderResourceView(nullptr, &srv_desc, m_srvNullHandle.CPUPtr);

				m_uavNullHandle = m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).GetNewHandle();
				D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
				uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				m_device->CreateUnorderedAccessView(nullptr, nullptr, &uav_desc, m_uavNullHandle.CPUPtr);

				m_samNullHandle = m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).GetNewHandle();
				D3D12_SAMPLER_DESC sampler_desc = {};
				sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				m_device->CreateSampler(&sampler_desc, m_samNullHandle.CPUPtr);

				m_swapchainImages.resize(RenderContext::Instance().GetFramesInFligtCount());
				for (size_t i = 0; i < RenderContext::Instance().GetFramesInFligtCount(); ++i)
				{
					m_swapchainImages[i].ColourHandle = m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).GetNewHandle();
				}

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
						context.DescriptorHeapGPURes.Create(DescriptorHeapTypes::CBV_SRV_UAV, 100000);
						context.DescriptorHeapSampler.SetRenderContext(this);
						context.DescriptorHeapSampler.Create(DescriptorHeapTypes::Sampler, 2048);
					});

				m_uploadQueue.Init();

				WaitForGpu();

				return true;
			}

			void RenderContext_DX12::Destroy()
			{
				WaitForGpu();
				m_resource_tracker.Release();

				m_rendererThreadShutdown = true;
				if (m_rendererThread.joinable())
				{
					m_rendererThread.join();
				}

				DestroyImGui();

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

				m_queues.clear();

				for (auto& image : m_swapchainImages)
				{
					Renderer::FreeTexture(image.Colour);
					image.Colour = nullptr;
				}

				m_samplerManager->ReleaseAll();

				BaseDestroy();

				m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).Destroy();

				if (m_swapchain)
				{
					m_swapchainImages.clear();
					m_swapchain.Reset();
					m_swapchain = nullptr;
				}

				if (m_d3d12MA)
				{
					m_d3d12MA->Release();
					m_d3d12MA = nullptr;
				}

				m_graphicsQueue.Release();

				if (m_device)
				{
					m_device.Reset();
					m_device = nullptr;
				}

				if (m_gpuCrashTracker)
				{
					m_gpuCrashTracker->Destroy();
					DeleteTracked(m_gpuCrashTracker);
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
					RenderContext::Instance().GetFramesInFligtCount(),
					DXGI_FORMAT_R8G8B8A8_UNORM,
					m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).GetHeap(handle.HeapId),
					handle.CPUPtr,
					handle.GPUPtr);

				ImGui_ImplDX12_NewFrame();
				ImGuiBeginFrame();
			}

			void RenderContext_DX12::DestroyImGui()
			{
				ImGuiRelease();
				ImGui_ImplDX12_Shutdown();
			}

			bool RenderContext_DX12::PrepareRender()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				{
					IS_PROFILE_SCOPE("ImGui Render");
					//ImGuiRender();
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
					//u64 fenceCompletedValue = m_submitFrameContexts.Get().SubmitFence->GetCompletedValue();
					//if (fenceCompletedValue < m_submitFrameContexts.Get().SubmitFenceValue)
					//{
					//	ThrowIfFailed(m_submitFrameContexts.Get().SubmitFence->SetEventOnCompletion(m_submitFrameContexts.Get().SubmitFenceValue, m_submitFrameContexts.Get().SubmitFenceEvent));
					//	WaitForSingleObjectEx(m_submitFrameContexts.Get().SubmitFenceEvent, INFINITE, FALSE);
					//}
					m_graphicsQueue.Wait();
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
				ExecuteAsyncJobs(cmdList);

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
				barrier.pResource = m_swapchainImages[m_availableSwapchainImage].Colour->m_swapchainImage.Get();
				cmdListDX12->PipelineBarrierImage({ barrier });
#else
				cmdListDX12->PipelineResourceBarriers({ CD3DX12_RESOURCE_BARRIER::Transition(
					m_swapchainImages[m_availableSwapchainImage].Colour->m_swapchainImage.Get(),
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
							//ID3D12CommandList* ppCommandLists[] = { cmdListDX12->GetCommandList() };
							//m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
							//
							//// Schedule a Signal command in the queue.
							//++m_submitFrameContexts.Get().SubmitFenceValue;
							//const UINT64 currentFenceValue = m_submitFrameContexts.Get().SubmitFenceValue;
							//ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_submitFrameContexts.Get().SubmitFence.Get(), currentFenceValue));

							m_graphicsQueue.Submit(cmdListDX12);

							++m_frameCount;
						}

						{
							IS_PROFILE_SCOPE("Present");
							// Present the frame.
							UINT presentSyncInterval = m_swapchainDesc.PresentMode == SwapchainPresentModes::Variable ? 0 : 1; 
							UINT presentFlags = m_swapchainDesc.PresentMode == SwapchainPresentModes::Variable && !Window::Instance().IsFullScreen() 
								? DXGI_PRESENT_ALLOW_TEARING : 0;
							
							if (HRESULT hr = m_swapchain->Present(presentSyncInterval, presentFlags); FAILED(hr))
							{
								if (hr == 0x0)
								{
									HRESULT deviceRemovedReason = m_device->GetDeviceRemovedReason();
									IS_CORE_ERROR("[RenderContext_DX12::PostRender] Device has been removed. Reason: '{}'.", deviceRemovedReason);
								}
							}

							m_currentFrame = (m_currentFrame + 1) % RenderContext::Instance().GetFramesInFligtCount();
						}
					}
				}
				m_frameIndex = (m_frameIndex + 1) % GetFramesInFligtCount();

				{
					IS_PROFILE_SCOPE("ImGui NewFrame");
					//ImGui_ImplDX12_NewFrame();
					//ImGuiBeginFrame();
				}

				m_resource_tracker.EndFrame();
			}

			void RenderContext_DX12::CreateSwapchain(SwapchainDesc desc)
			{
				if (m_swapchain)
				{
					for (auto& image : m_swapchainImages)
					{
						image.Colour->m_swapchainImage.Reset();
						image.Colour->m_swapchainImage = nullptr;
					}
					m_swapchain.Reset();
					m_swapchain = nullptr;
				}

				IDXGIFactory4* factory = m_factory.Get();
				ID3D12Device* device = m_device.Get();

				// Swap chain can not be 0.
				desc.Width = std::max(1u, desc.Width);
				desc.Height = std::max(1u, desc.Height);
				m_swapchainBufferSize = { desc.Width, desc.Height };

				BOOL allowTearing = FALSE;
				bool tearingSupported = FALSE;

				ComPtr<IDXGIFactory5> dxgiFactory5;
				HRESULT hr = m_factory.As(&dxgiFactory5);
				if (SUCCEEDED(hr))
				{
					hr = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
				}

				if (FAILED(hr) || !allowTearing)
				{
					tearingSupported = false;
					IS_CORE_WARN("[RenderContext_DX12::CreateSwapchain]: Variable refresh rate displays not supported. Fallback to VSync.\n");
					desc.PresentMode = SwapchainPresentModes::VSync;
				}
				else
				{
					tearingSupported = true;
				}

				UINT swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
				if (desc.PresentMode == SwapchainPresentModes::Variable 
					&& tearingSupported)
				{
					swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				}

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.BufferCount = RenderContext::Instance().GetFramesInFligtCount();
				swapChainDesc.Width = static_cast<UINT>(m_swapchainBufferSize.x);
				swapChainDesc.Height = static_cast<UINT>(m_swapchainBufferSize.y);
				swapChainDesc.Format = PixelFormatToDX12(desc.Format);
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = SwapchainPresentModeToDX12(desc.PresentMode);
				swapChainDesc.Stereo = false;
				swapChainDesc.Flags = swapChainFlags;

				GLFWwindow* window = Window::Instance().GetRawWindow();
				HWND hwmd = glfwGetWin32Window(window);

				ComPtr<IDXGISwapChain1> swapchain;
				ThrowIfFailed(factory->CreateSwapChainForHwnd(
					m_graphicsQueue.GetQueue(),
					hwmd,
					&swapChainDesc,
					nullptr,
					nullptr,
					&swapchain));

				if (tearingSupported)
				{
					// When tearing support is enabled we will handle ALT+Enter key presses in the
					// window message loop rather than let DXGI handle it by calling SetFullscreenState.
					m_factory->MakeWindowAssociation(hwmd, DXGI_MWA_NO_ALT_ENTER);
				}
				else
				{
					if (Window::Instance().IsFullScreen())
					{
						BOOL fullscreenState;
						ThrowIfFailed(swapchain->GetFullscreenState(&fullscreenState, nullptr));
						if (FAILED(swapchain->SetFullscreenState(!fullscreenState, nullptr)))
						{
							// Transitions to fullscreen mode can fail when running apps over
							// terminal services or for some other unexpected reason.  Consider
							// notifying the user in some way when this happens.
							IS_CORE_ERROR("[RenderContext_DX12::CreateSwapchain] Fullscreen transition failed.");
							FAIL_ASSERT();
						}
					}
				}

				swapchain.As(&m_swapchain);

				//m_swapchainImages.resize(RenderContext::Instance().GetFramesInFligtCount());
				for (u32 i = 0; i < RenderContext::Instance().GetFramesInFligtCount(); ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];

					if (!swapchainImage.Colour)
					{
						RHI_Texture* tex = Renderer::CreateTexture();
						tex->SetName("Swapchain_Image: " + std::to_string(i));
						RHI_Texture_DX12* textureDX12 = static_cast<RHI_Texture_DX12*>(tex);
						textureDX12->m_context = this;
						swapchainImage.Colour = textureDX12;
						textureDX12->m_allLayerRenderTargetHandle = swapchainImage.ColourHandle;
						textureDX12->m_singleLayerRenderTargetHandle.push_back(swapchainImage.ColourHandle);

						RHI_TextureInfo textureInfo = {};
						textureInfo.TextureType = TextureType::Tex2D;
						textureInfo.Width = swapChainDesc.Width;
						textureInfo.Height = swapChainDesc.Height;
						textureInfo.Depth = 1;
						textureInfo.Format = desc.Format;
						textureInfo.ImageUsage = 0;
						textureInfo.Layout = ImageLayout::PresentSrc;
						textureInfo.InitalStatus = DeviceUploadStatus::Completed;
						textureDX12->m_infos.push_back(textureInfo);
					}
					swapchainImage.Colour->SetName("Swapchain_" + std::to_string(i));

					/// Get the back buffer from the swapchain.
					ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainImage.Colour->m_swapchainImage)));
					device->CreateRenderTargetView(swapchainImage.Colour->m_swapchainImage.Get(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());
				}
				m_swapchainDesc = desc;
			}

			void RenderContext_DX12::SetSwaphchainResolution(glm::ivec2 resolution)
			{
				m_gpu_defered_manager.Instance().Push(this, [this, resolution](RHI_CommandList* cmdList)
					{
						WaitForGpu();

						SwapchainDesc desc = m_swapchainDesc;
						desc.Width = resolution.x;
						desc.Height = resolution.y;

						CreateSwapchain(desc);
						Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
					});
			}

			glm::ivec2 RenderContext_DX12::GetSwaphchainResolution() const
			{
				return m_swapchainBufferSize;
			}

			void RenderContext_DX12::SetFullScreen()
			{
				//m_swapchain->SetFullscreenState(TRUE, nullptr);
				SetSwaphchainResolution(Window::Instance().GetSize());
			}

			void RenderContext_DX12::GpuWaitForIdle()
			{
				WaitForGpu();
			}

			void RenderContext_DX12::SubmitCommandListAndWait(RHI_CommandList* cmdList)
			{
				const RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(cmdList);
				//ID3D12CommandList* ppCommandLists[] = { cmdListDX12->GetCommandList() };
				//m_queues[GPUQueue_Graphics]->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				//GpuWaitForIdle();
				m_graphicsQueue.SubmitAndWait(cmdListDX12);
			}

			void RenderContext_DX12::ExecuteAsyncJobs(RHI_CommandList* cmdList)
			{
				// Go through out deferred manager and call all the functions which have been queued up.
				m_gpu_defered_manager.Update(cmdList);
				m_uploadQueue.UploadToDevice(cmdList);
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
							continue;
						}

						/// Check to see whether the adapter supports Direct3D 12, but don't create the
						/// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
						{
							m_d3dFeatureLevel = D3D_FEATURE_LEVEL_12_0;
							break;
						}

						if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
						{
							m_d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
							break;
						}

						if (m_d3dFeatureLevel == D3D_FEATURE_LEVEL_1_0_CORE)
						{
							FAIL_ASSERT();
						}
					}
				}

				ASSERT(m_d3dFeatureLevel != D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_1_0_CORE);
#if false
				if (adapter == nullptr)
				{
					for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
						{
							/// Don't select the Basic Render Driver adapter.
							continue;
						}

						/// Check to see whether the adapter supports Direct3D 12, but don't create the
						/// actual device yet.
						if (SUCCEEDED(D3D12CreateDevice(nullptr, m_d3dFeatureLevel, __uuidof(ID3D12Device), nullptr)))
						{
							break;
						}
					}
				}
#endif

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

			void RenderContext_DX12::ResizeSwapchainBuffers()
			{
				/// Release all our previous render targets from the swapchain.
				for (size_t i = 0; i < m_swapchainImages.size(); ++i)
				{
					m_swapchainImages[i].Colour->m_swapchainImage.Reset();
					m_swapchainImages[i].Colour->m_swapchainImage = nullptr;
				}

				/// Resize our swap chain buffers.
				m_swapchain->ResizeBuffers(RenderContext::Instance().GetFramesInFligtCount(), m_swapchainBufferSize.x, m_swapchainBufferSize.y, DXGI_FORMAT_UNKNOWN, 0);
				const UINT frameIndex = m_swapchain->GetCurrentBackBufferIndex();

				/// Create new render targets for the swapchain.
				for (u32 i = 0; i < RenderContext::Instance().GetFramesInFligtCount(); ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];
					ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainImage.Colour->m_swapchainImage)));
					m_device->CreateRenderTargetView(swapchainImage.Colour->m_swapchainImage.Get(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());
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

			DescriptorHeapHandle_DX12 RenderContext_DX12::GetDescriptorCBVNullHandle() const
			{
				return m_cbvNullHandle;
			}

			DescriptorHeapHandle_DX12 RenderContext_DX12::GetDescriptorSRVNullHandle() const
			{
				return m_srvNullHandle;
			}

			DescriptorHeapHandle_DX12 RenderContext_DX12::GetDescriptorUAVNullHandle() const
			{
				return m_uavNullHandle;
			}

			DescriptorHeapHandle_DX12 RenderContext_DX12::GetDescriptorSAMNullHandle() const
			{
				return m_samNullHandle;
			}

			void RenderContext_DX12::WaitForGpu()
			{
				// Increment the fence value for the current frame.
				//++m_submitFrameContexts.Get().SubmitFenceValue;
				//u64 currentFenceValue = m_submitFrameContexts.Get().SubmitFence->GetCompletedValue();
				//
				//// Schedule a Signal command in the queue.
				//ThrowIfFailed(m_queues[GPUQueue_Graphics]->Signal(m_submitFrameContexts.Get().SubmitFence.Get(), m_submitFrameContexts.Get().SubmitFenceValue));
				//
				//// Wait until the fence has been processed.
				//ThrowIfFailed(m_submitFrameContexts.Get().SubmitFence->SetEventOnCompletion(m_submitFrameContexts.Get().SubmitFenceValue, m_submitFrameContexts.Get().SubmitFenceEvent));
				//WaitForSingleObjectEx(m_submitFrameContexts.Get().SubmitFenceEvent, INFINITE, FALSE);
				//
				//currentFenceValue = m_submitFrameContexts.Get().SubmitFence->GetCompletedValue();
				m_graphicsQueue.SignalAndWait();
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)