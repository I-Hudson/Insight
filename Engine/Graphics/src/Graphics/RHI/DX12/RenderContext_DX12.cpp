#if defined(IS_DX12_ENABLED)

#ifdef IS_PLATFORM_WINDOWS

#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/Window.h"

#include "Graphics/RHI/RHI_GPUCrashTracker.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Platforms/Platform.h"

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
					// Enable the debug layer (requires the Graphics Tools "optional feature").
					// NOTE: Enabling the debug layer after device creation will invalidate the active device.
					{
						if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
						{
							m_debugController->EnableDebugLayer();
							ID3D12Debug1* spDebugController1;
							if (SUCCEEDED(m_debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1))))
							{
								spDebugController1->SetEnableGPUBasedValidation(true);
							}

							/// Enable additional debug layers.
							dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

						}
					}
					//#endif
				}

				//EnabledRenderOption(RenderOptions::ReverseZ);

				ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
				if (!m_factory)
				{
					return false;
				}

				FindPhysicalDevice(&m_physicalDevice.GetPhysicalDevice());

				ThrowIfFailed(D3D12CreateDevice(
					m_physicalDevice.GetPhysicalDevice().Get(),
					m_d3dFeatureLevel,
					IID_PPV_ARGS(&m_device)
				));

				if (!m_desc.GPUValidation && !RenderDocAPI::Instance().IsInitialised())
				{
					m_gpuCrashTracker = RHI_GPUCrashTracker::Create();
					if (m_gpuCrashTracker)
					{
						m_gpuCrashTracker->Init();
					}
				}

				SetDeviceExtenstions();

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
					IS_LOG_CORE_WARN("[GPUDevice_DX12::Init] Queue not supproted: {}, HR: {}",
						(int)D3D12_COMMAND_LIST_TYPE_COMPUTE, HrToString(createCommandQueueResult));
				}

				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
				createCommandQueueResult = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[GPUQueue_Transfer]));
				if (createCommandQueueResult != S_OK)
				{
					IS_LOG_CORE_WARN("[GPUDevice_DX12::Init] Queue not supproted: {}, HR: {}",
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

				D3D12_QUERY_HEAP_DESC timeStampDesc = {};
				timeStampDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
				timeStampDesc.NodeMask = 0;
				timeStampDesc.Count = static_cast<u32>(m_timeStampQueryMaxCountPerFrame * 2);
				ThrowIfFailed(m_device->CreateQueryHeap(&timeStampDesc, IID_PPV_ARGS(&m_timeStampQueryHeap)));

				m_timeStampReadbackBuffer = Renderer::CreateReadbackBuffer(timeStampDesc.Count * sizeof(u64));

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
						context.DescriptorHeapGPURes.Create(DescriptorHeapTypes::CBV_SRV_UAV, 100000, "CBV_SRV_UAV_HEAP");
						context.DescriptorHeapSampler.SetRenderContext(this);
						context.DescriptorHeapSampler.Create(DescriptorHeapTypes::Sampler, 2048, "SAMPLER_HEAP");
					});

				m_submitFenceValues.Setup();
				m_submitFenceValues.ForEach([](u64& fenceValue)
					{
						fenceValue = 0;
					});

				m_uploadQueue.Init();

				WaitForGpu();

				if (desc.MultithreadContext)
				{
					StartRenderThread();
				}

				return true;
			}

			void RenderContext_DX12::Destroy()
			{
				IS_PROFILE_FUNCTION();

				StopRenderThread();
				WaitForGpu();

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

				m_timeStampQueryHeap->Release();
				m_timeStampQueryHeap = nullptr;

				Renderer::FreeReadbackBuffer(m_timeStampReadbackBuffer);
				m_timeStampReadbackBuffer = nullptr;

				BaseDestroy();
				m_resource_tracker.Release();

				m_descriptorHeaps.at(DescriptorHeapTypes::CBV_SRV_UAV).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::Sampler).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::RenderTargetView).Destroy();
				m_descriptorHeaps.at(DescriptorHeapTypes::DepthStencilView).Destroy();

				if (m_swapchain)
				{
					IS_PROFILE_SCOPE("Swapchain Release");
					m_swapchainImages.clear();
					m_swapchain.Reset();
					m_swapchain = nullptr;
				}

				if (m_d3d12MA)
				{
					IS_PROFILE_SCOPE("D3D12MA Release");
					m_d3d12MA->Release();
					m_d3d12MA = nullptr;
				}

				m_graphicsQueue.Release();

				if (m_device)
				{
					IS_PROFILE_SCOPE("Device Release");
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
					IS_PROFILE_SCOPE("Factory Release");
					m_factory.Reset();
					m_factory = nullptr;
				}
				m_physicalDevice = {};

				ComPtr<IDXGIDebug1> dxgiDebug;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
				{
					IS_PROFILE_SCOPE("ReportLiveObjects");
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
				IS_PROFILE_FUNCTION();

				ImGuiRelease();
				ImGui_ImplDX12_Shutdown();
			}

			bool RenderContext_DX12::PrepareRender()
			{
				IS_PROFILE_FUNCTION();
				std::lock_guard lock(m_lock);

				RHI_MemoryInfo& rhiMemoryInfo = m_rhiMemoryInfo.Get();
				D3D12MA::Budget pLocalBudget;
				D3D12MA::Budget pNonLocalBudget;
				m_d3d12MA->GetBudget(&pLocalBudget, &pNonLocalBudget);
				Platform::MemCopy(&rhiMemoryInfo, &pLocalBudget, sizeof(pLocalBudget));

				{
					IS_PROFILE_SCOPE("Fence wait");
					m_graphicsQueue.Wait(m_submitFenceValues.Get());
					m_submitFrameContexts->OnCompleted();
					m_frameIndexCompleted.store(m_frameIndex.load());
				}

				{
					nvtx3::scoped_range fenceWaitRange{"PrepareRender Reset"};
					m_descriptorSetManager->Reset();
					m_commandListManager->Reset();
					m_resource_tracker.BeginFrame();
				}

				if (Window::Instance().GetWidth() == 0 || Window::Instance().GetHeight() == 0)
				{
					return false;
				}

				if (Window::Instance().GetSize() != m_swapchainBufferSize)
				{
					IS_PROFILE_SCOPE("Swapchain resize");
					SetSwaphchainResolution(Maths::Vector2(Window::Instance().GetWidth(), Window::Instance().GetHeight()));
					return false;
				}

				return true;
			}

			void RenderContext_DX12::PreRender(RHI_CommandList* cmdList)
			{
				ExecuteAsyncJobs(cmdList);
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
							m_graphicsQueue.Submit(cmdListDX12);
						}

						{
							IS_PROFILE_SCOPE("Present");
							// Present the frame.
							UINT presentSyncInterval = m_swapchainDesc.PresentMode == SwapchainPresentModes::Variable ? 0 : 1; 
							UINT presentFlags = m_swapchainDesc.PresentMode == SwapchainPresentModes::Variable && !Window::Instance().IsFullScreen() 
								? DXGI_PRESENT_ALLOW_TEARING : 0;
							
							if (HRESULT hr = m_swapchain->Present(presentSyncInterval, presentFlags); FAILED(hr))
							{
								if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
								{
									HRESULT removeReason = m_device->GetDeviceRemovedReason();
									if (m_gpuCrashTracker)
									{
										m_gpuCrashTracker->DeviceLost();
									}

									HRESULT deviceRemovedReason = m_device->GetDeviceRemovedReason();
									IS_LOG_CORE_ERROR("[RenderContext_DX12::PostRender] Device has been removed. Reason: '{}'.", deviceRemovedReason);
								}
							}

							m_currentFrame = (m_currentFrame + 1) % RenderContext::Instance().GetFramesInFligtCount();
						}
					}
				}
				m_resource_tracker.EndFrame();
				
				m_submitFenceValues.Get() = m_graphicsQueue.Signal();
				m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
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
				m_swapchainBufferSize = Maths::Vector2(desc.Width, desc.Height);

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
					IS_LOG_CORE_WARN("[RenderContext_DX12::CreateSwapchain]: Variable refresh rate displays not supported. Fallback to VSync.\n");
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
							IS_LOG_CORE_ERROR("[RenderContext_DX12::CreateSwapchain] Fullscreen transition failed.");
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
				m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();
			}

			void RenderContext_DX12::SetSwaphchainResolution(Maths::Vector2 resolution)
			{
				m_gpu_defered_manager.Instance().Push([this, resolution](RHI_CommandList* cmdList)
					{
						WaitForGpu();

						SwapchainDesc desc = m_swapchainDesc;
						desc.Width = resolution.x;
						desc.Height = resolution.y;

						CreateSwapchain(desc);
						Core::EventSystem::Instance().DispatchEvent(MakeRPtr<Core::GraphcisSwapchainResize>(m_swapchainBufferSize.x, m_swapchainBufferSize.y));
					});
			}

			Maths::Vector2 RenderContext_DX12::GetSwaphchainResolution() const
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
				cmdList->OnWorkCompleted();
			}

			void RenderContext_DX12::MarkTimeStamp(RHI_CommandList* cmdList)
			{
				if (m_timeStampCurrentCount == m_timeStampQueryMaxCountPerFrame)
				{
					return;
				}

				const RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(cmdList);
				const u32 timeStampIndex = static_cast<u32>(m_timeStampCurrentIndex + m_timeStampCurrentCount);
				cmdListDX12->GetCommandList()->EndQuery(m_timeStampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timeStampIndex);
				++m_timeStampCurrentCount;
			}

			std::vector<u64> RenderContext_DX12::ResolveTimeStamps(RHI_CommandList* cmdList)
			{
				cmdList->BeginTimeBlock("ResolveTimestamp queries");
				const RHI_CommandList_DX12* cmdListDX12 = static_cast<RHI_CommandList_DX12*>(cmdList);
				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(m_timeStampReadbackBuffer);
				// Resolve the data
				const u64 dstOffset = (m_timeStampCurrentIndex * 8);
				const u32 timeStampIndex = static_cast<u32>(m_timeStampCurrentIndex);
				const u32 timeStampCount = static_cast<u32>(m_timeStampPreviousCount);
				cmdListDX12->GetCommandList()->ResolveQueryData(m_timeStampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timeStampIndex, timeStampCount, bufferDX12->GetResource(), dstOffset);

				if (!m_timeStampEnoughFrames)
				{
					// We need to wait until we have more than a single frame.
					m_timeStampEnoughFrames = true;
					m_timeStampPreviousCount = m_timeStampCurrentCount;
					m_timeStampCurrentCount = 0;

					m_timeStampCurrentIndex = (m_timeStampCurrentIndex + m_timeStampQueryMaxCountPerFrame) % (m_timeStampQueryMaxCountPerFrame * 2);
					cmdList->EndTimeBlock();
					return {};
				}

				m_timeStampCurrentIndex = (m_timeStampCurrentIndex + m_timeStampQueryMaxCountPerFrame) % (m_timeStampQueryMaxCountPerFrame * 2);

				std::vector<u8> bufferData = RemoveConst(bufferDX12)->Download();
				std::vector<u64> queryData;
				queryData.resize(m_timeStampPreviousCount);
				Platform::MemCopy(queryData.data(), bufferData.data() + (m_timeStampCurrentIndex * 8), queryData.size() * sizeof(u64));

				m_timeStampPreviousCount = m_timeStampCurrentCount;
				m_timeStampCurrentCount = 0;

				cmdList->EndTimeBlock();
				return queryData;
			}

			u64 RenderContext_DX12::GetTimeStampFrequency()
			{
				u64 frequency = 0;
				ThrowIfFailed(m_graphicsQueue.GetQueue()->GetTimestampFrequency(&frequency));
				return frequency;
			}

			void RenderContext_DX12::ExecuteAsyncJobs(RHI_CommandList* cmdList)
			{
				if (cmdList == nullptr)
				{
					return;
				}

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
					u32 adapterIdx = 0;
					u32 vram = 0;

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

						const auto findDXFeatureLevel = [&](const D3D_FEATURE_LEVEL featureLevel)
							{
								/// Check to see whether the adapter supports Direct3D 12, but don't create the
								/// actual device yet.
								if (SUCCEEDED(D3D12CreateDevice(nullptr, featureLevel, __uuidof(ID3D12Device), nullptr))
									&& m_d3dFeatureLevel <= featureLevel
									&& desc.DedicatedVideoMemory > vram)
								{
									m_d3dFeatureLevel = featureLevel;
									adapterIdx = adapterIndex;
									vram = desc.DedicatedVideoMemory;
								}
							};

						findDXFeatureLevel(D3D_FEATURE_LEVEL_12_0);
						findDXFeatureLevel(D3D_FEATURE_LEVEL_11_0);
						findDXFeatureLevel(D3D_FEATURE_LEVEL_1_0_CORE);
					}

					SUCCEEDED(factory6->EnumAdapterByGpuPreference(
						adapterIdx,
						DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
						IID_PPV_ARGS(&adapter)));
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

			void RenderContext_DX12::SetDeviceExtenstions()
			{
				D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
				ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));

				D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1{};
				ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1)));

				D3D12_FEATURE_DATA_D3D12_OPTIONS2 options2{};
				ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &options2, sizeof(options2)));

				D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3{};
				ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &options3, sizeof(options3)));

				m_deviceExtensions[(u8)DeviceExtension::BindlessDescriptors] = options.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_3;
				m_deviceExtensions[(u8)DeviceExtension::ExclusiveFullScreen] = true;
				m_deviceExtensions[(u8)DeviceExtension::VulkanDynamicRendering] = false;
				m_deviceExtensions[(u8)DeviceExtension::FormatTypeCasting] = options3.CastingFullyTypedFormatSupported;
				m_deviceExtensions[(u8)DeviceExtension::Native16BitOps] = options.MinPrecisionSupport == D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT;
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
				m_swapchain->ResizeBuffers(RenderContext::Instance().GetFramesInFligtCount(), static_cast<u32>(m_swapchainBufferSize.x), static_cast<u32>(m_swapchainBufferSize.y), DXGI_FORMAT_UNKNOWN, 0);
				const UINT frameIndex = m_swapchain->GetCurrentBackBufferIndex();

				/// Create new render targets for the swapchain.
				for (u32 i = 0; i < RenderContext::Instance().GetFramesInFligtCount(); ++i)
				{
					SwapchainImage& swapchainImage = m_swapchainImages[i];
					ThrowIfFailed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainImage.Colour->m_swapchainImage)));
					m_device->CreateRenderTargetView(swapchainImage.Colour->m_swapchainImage.Get(), nullptr, swapchainImage.ColourHandle.GetCPUHandle());
				}
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

#ifdef IS_RESOURCE_HANDLES_ENABLED
			RHI_Handle<Texture> RenderContext_DX12::CreateTexture(const Texture texture)
			{
				TextureDrawData_DX12 drawData;
				Texture textureData;
				RHI_Handle<Texture> handle = m_texturePool.Create(drawData, textureData);

				textureData = texture;
				drawData.LayerCount = textureData.LayerCount;
				Platform::MemCopy(drawData.ClearColour, textureData.ClearColour, sizeof(textureData.ClearColour));

				CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					PixelFormatToDX12(textureData.Format),
					textureData.Width,
					textureData.Height,
					textureData.LayerCount,
					textureData.MipCount,
					1,
					0,
					ImageUsageFlagsToDX12(textureData.ImageUsage),
					D3D12_TEXTURE_LAYOUT_UNKNOWN,
					0);

				D3D12_CLEAR_VALUE clearColour = {};
				clearColour.Format = resourceDesc.Format;
				if (textureData.ImageUsage & ImageUsageFlagsBits::DepthStencilAttachment)
				{
					clearColour.DepthStencil.Depth = RenderContext::Instance().IsRenderOptionsEnabled(RenderOptions::ReverseZ) ? 0.0f : 1.0f;
					clearColour.DepthStencil.Stencil = 0;
				}
				else if (textureData.ImageUsage & ImageUsageFlagsBits::ColourAttachment)
				{
					Platform::MemCopy(clearColour.Color, textureData.ClearColour, sizeof(textureData.ClearColour));
				}

				D3D12MA::ALLOCATION_DESC allocationDesc = {};
				allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

				ThrowIfFailed(GetAllocator()->CreateResource(
					&allocationDesc,
					&resourceDesc,
					ImageLayoutToDX12ResouceState(textureData.Layout),
					&clearColour,
					&drawData.D3D12Allocation,
					IID_NULL, NULL));
				if (!textureData.DebugName.empty())
				{
					SetObjectName(textureData.DebugName.c_str(), drawData.D3D12Allocation->GetResource());
				}
				ASSERT(drawData.D3D12Allocation);

				return handle;
			}

			void RenderContext_DX12::FreeTexture(const RHI_Handle<Texture> handle)
			{
				TextureDrawData_DX12 drawData;
				Texture textureData;
				m_texturePool.Release(handle, drawData, textureData);

				RHI_ResourceRenderTracker::Instance().AddDeferedRelase([drawData]()
					{
						drawData.D3D12Allocation->Release();
					});
			}

			void RenderContext_DX12::UploadToTexture(const RHI_Handle<Texture> handle, const std::vector<u8>& data)
			{
				GetUploadQueue().UploadTexture(data.data(), data.size(), handle);
			}

			Texture* RenderContext_DX12::GetTexture(const RHI_Handle<Texture> handle) const
			{
				return const_cast<Texture*>(m_texturePool.GetColdType(handle));
			}
#endif

			void RenderContext_DX12::WaitForGpu()
			{
				IS_PROFILE_FUNCTION();

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