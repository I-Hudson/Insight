#pragma once

#include "Graphics/Defines.h"
#include "Graphics/Enums.h"
#include "Graphics/GPUDeferedManager.h"
#include "imgui.h"

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/RHI_Renderpass.h"
#include "Graphics/RHI/RHI_Sampler.h"
#include "Graphics/RHI/RHI_ResourceRenderTracker.h"
#include "Graphics/RHI/RHI_UploadQueue.h"
#include "Graphics/RHI/RHI_PipelineManager.h"
#include "Graphics/RHI/RHI_MemoryInfo.h"

#include "Graphics/RenderStats.h"
#include "Graphics/GPUProfiler.h"
#include "Graphics/RenderDocAPI.h"

#ifdef IS_RESOURCE_HANDLES_ENABLED
#include "Graphics/Resources/Texture.h"
#include "Graphics/RHI/RHI_Handle.h"
#include "Graphics/RHI/RHI_ResourcePool.h"
#endif

#include "Threading/Semaphore.h"

#include <mutex>

namespace Insight
{
	class Renderer;
	
	namespace Graphics
	{
		class RenderContext;
		class RHI_Texture;
		class RenderTarget;
		class RenderGraph;
		class RenderGraphV2;

		enum class GraphicsAPI
		{
			Vulkan,
			DX12,

			None,
		};
		constexpr const char* GraphicsAPIStrings[] = { "Vulkan", "DX12", "None" };
		static_assert(ARRAY_COUNT(GraphicsAPIStrings) == (static_cast<u64>(GraphicsAPI::None) + 1));

		IS_GRAPHICS constexpr const char* GraphicsAPIToString(GraphicsAPI api)
		{
			return GraphicsAPIStrings[static_cast<u32>(api)];
		}

		//// <summary>
		//// Store any and all details about the physical device being used.
		//// </summary>
		struct IS_GRAPHICS PhysicalDeviceInformation : public Core::Singleton<PhysicalDeviceInformation>
		{
			std::string Device_Name;
			std::string Vendor;
			u32 Vendor_Id = 0;
			u64 VRam_Size;

			u64 MinUniformBufferAlignment = 0;

			void SetVendorName()
			{
				ASSERT(Vendor_Id != 0);
				switch (Vendor_Id)
				{
				case 0x1002: Vendor = "AMD"; break;
				case 0x8086: Vendor = "INTEL"; break;
				case 0x10DE: Vendor = "NVIDIA"; break;
				case 0x1414: Vendor = "MICROSOFT"; break;
				default:
					break;
				}
			}
		};

		template<typename TValue>
		class FrameResource
		{
		public:
			void Setup()
			{
				m_values.clear();
				m_values.resize(RenderContext::Instance().GetFramesInFligtCount());
			}

			TValue* operator->() const
			{
				return const_cast<TValue*>(&m_values.at(RenderContext::Instance().GetFrameIndex()));
			}

			TValue& Get()
			{
				ASSERT(!m_values.empty());
				return m_values.at(RenderContext::Instance().GetFrameIndex());
			}
			const TValue& Get() const
			{
				ASSERT(!m_values.empty());
				return m_values.at(RenderContext::Instance().GetFrameIndex());
			}

			TValue& GetRenderCompeted()
			{
				ASSERT(!m_values.empty());
				return m_values.at(RenderContext::Instance().GetFrameIndexCompleted());
			}
			const TValue& GetRenderCompeted() const
			{
				ASSERT(!m_values.empty());
				return m_values.at(RenderContext::Instance().GetFrameIndexCompleted());
			}

			u64 Size() const
			{
				return m_values.size();
			}

			void ForEach(std::function<void(TValue& value)> func)
			{
				for (TValue& v : m_values)
				{
					func(v);
				}
			}

		private:
			std::vector<TValue> m_values;
		};

		/// @brief Define values for the swapchain.
		struct SwapchainDesc
		{
			u32 Width;
			u32 Height;

			PixelFormat Format;

			SwapchainPresentModes PresentMode = SwapchainPresentModes::Variable;

			bool AllowHDR = false;
		};

		/// @brief Define general options for the render context.
		struct RenderContextDesc
		{
			bool GPUValidation = true;
			bool MultithreadContext = false;
		};

		class IS_GRAPHICS RenderContext : public Core::Singleton<RenderContext>
		{
		public:
			RenderContext();
			virtual ~RenderContext() = default;

			static RenderContext* New(GraphicsAPI graphicsAPI);

			void Render();
			bool IsRenderThread() const;

			virtual bool Init(RenderContextDesc desc) = 0;
			virtual void Destroy() = 0;

			virtual void InitImGui() = 0;
			virtual void DestroyImGui() = 0;

			virtual bool PrepareRender() = 0;
			virtual void PreRender(RHI_CommandList* cmdList) = 0;
			virtual void PostRender(RHI_CommandList* cmdList) = 0;

			virtual void CreateSwapchain(SwapchainDesc desc) = 0;
			virtual void SetSwaphchainResolution(Maths::Vector2 resolution) = 0;
			virtual Maths::Vector2 GetSwaphchainResolution() const = 0;

			virtual void GpuWaitForIdle() = 0;
			virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) = 0;

			virtual void MarkTimeStamp(RHI_CommandList* cmdList) = 0;
			virtual std::vector<u64> ResolveTimeStamps(RHI_CommandList* cmdList) = 0;
			virtual u64 GetTimeStampFrequency() = 0;

			/// @brief Execute anything that is not directly graphics related like uploading data to the GPU.
			virtual void ExecuteAsyncJobs(RHI_CommandList* cmdList) = 0;

			virtual RHI_Texture* GetSwaphchainIamge() const = 0;

			virtual void SetFullScreen() { }

			u32 GetFrameIndex() const;
			u32 GetFrameIndexCompleted() const;
			u64 GetFrameCount() const;
			u32 GetFramesInFligtCount() const;

			void WaitForRenderThread();

			bool HasExtension(DeviceExtension extension) const;
			bool IsExtensionEnabled(DeviceExtension extension) const;
			void EnableExtension(DeviceExtension extension);
			void DisableExtension(DeviceExtension extension);
			GraphicsAPI GetGraphicsAPI() const { return m_graphicsAPI; }

			bool IsRenderOptionsEnabled(RenderOptions option) const;
			void EnabledRenderOption(RenderOptions option);
			void DisableRenderOption(RenderOptions option);

			CommandListManager& GetCommandListManager()					{ return m_commandListManager.Get(); }
			RHI_DescriptorLayoutManager& GetDescriptorLayoutManager()	{ return m_descriptorLayoutManager; }
			RHI_ShaderManager& GetShaderManager()						{ return m_shaderManager; }
			RHI_RenderpassManager& GetRenderpassManager()				{ return m_renderpassManager; }
			RHI_DescriptorSetManager& GetDescriptorSetManager()			{ return m_descriptorSetManager.Get(); }
			RHI_SamplerManager& GetSamplerManager()						{ return *m_samplerManager; }
			RHI_ResourceRenderTracker& GetResourceRenderTracker()		{ return m_resource_tracker; }

			RHI_PipelineManager& GetPipelineManager()					{ return m_pipelineManager; }
			RHI_PipelineLayoutManager& GetPipelineLayoutManager()		{ return m_pipelineLayoutManager; }

			GPUDeferedManager& GetDeferredManager()						{ return m_gpu_defered_manager; }
			RHI_UploadQueue& GetUploadQueue()							{ return m_uploadQueue; }

			RHI_MemoryInfo GetVRamInfo() const							{ return m_rhiMemoryInfo.GetRenderCompeted(); }

#ifdef IS_RESOURCE_HANDLES_ENABLED
			virtual RHI_Handle<Texture> CreateTexture(const Texture texture) { FAIL_ASSERT(); return {}; }
			virtual void FreeTexture(const RHI_Handle<Texture> handle) { FAIL_ASSERT(); }
			virtual void UploadToTexture(const RHI_Handle<Texture> handle, const std::vector<u8>& data) { FAIL_ASSERT(); }
			virtual Texture* GetTexture(const RHI_Handle<Texture> handle) const { FAIL_ASSERT(); return nullptr; }
#endif

			void ImGuiBeginFrame();
			void ImGuiRender();
		protected:
			void ImGuiRelease();

			virtual void WaitForGpu() = 0;

			void BaseDestroy();

			void RenderUpdateLoop();
			void StartRenderThread();
			void StopRenderThread();

		private:
			RHI_Buffer* CreateBuffer(BufferType bufferType, u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides);
			void FreeBuffer(RHI_Buffer* buffer);
			int GetBufferCount(BufferType bufferType) const;

			RHI_Texture* CreateTextre();
			void FreeTexture(RHI_Texture* texture);
			bool HasTexture(RHI_Texture* texture) const;

		protected:
			///const static int c_FrameCount = 3;

			std::mutex m_lock;
			RenderContextDesc m_desc;
			SwapchainDesc m_swapchainDesc;

			GraphicsAPI m_graphicsAPI = GraphicsAPI::None;
			
			std::thread::id m_renderThreadId;
			std::thread m_renderThread;
			std::atomic<bool> m_stopRenderThread;

			Semaphore m_renderTriggerSemaphore;
			Semaphore m_renderCompletedSemaphore;

			GPUProfiler m_gpuProfiler;

			RenderGraph* m_renderGraph;
			RenderGraphV2* m_renderGraphV2;

			std::array<u8, static_cast<u64>(DeviceExtension::Size)> m_deviceExtensions;
			std::array<u8, static_cast<u64>(DeviceExtension::Size)> m_enabledDeviceExtensions;
			std::array<u8, static_cast<u64>(RenderOptions::Size)> m_renderOptions;

			std::atomic<u32> m_framesInFlightCount = 2;
			/// @brief The current frame from 0 to c_FrameCount.
			std::atomic<u32> m_frameIndex = 0;
			std::atomic<u32> m_frameIndexCompleted = 0;
			/// @brief Current frame count for the whole life time of the app.
			std::atomic<u64> m_frameCount = 0;

			std::vector<IRHI_ResourceCache*> m_resourceCaches;
			RHI_ResourceManager<RHI_Texture> m_textures;
			RHI_ResourceManager<RHI_Buffer> m_buffers;

			RHI_ResourceRenderTracker m_resource_tracker;
			RHI_ShaderManager m_shaderManager;
			RHI_RenderpassManager m_renderpassManager;
			RHI_SamplerManager* m_samplerManager;
			GPUDeferedManager m_gpu_defered_manager;
			RHI_UploadQueue m_uploadQueue;

			RHI_PipelineManager m_pipelineManager;
			RHI_PipelineLayoutManager m_pipelineLayoutManager;

			FrameResource<RHI_MemoryInfo> m_rhiMemoryInfo;

			FrameResource<DescriptorAllocator> m_frameDescriptorAllocator;

			FrameResource<CommandListManager> m_commandListManager;
			FrameResource<RHI_DescriptorSetManager> m_descriptorSetManager;

			RHI_DescriptorLayoutManager m_descriptorLayoutManager;

			PhysicalDeviceInformation m_physical_device_info;
			RenderStats m_renderStats;
			RenderDocAPI m_renderDocAPI;

			Maths::Vector2 m_swapchainBufferSize;

			RHI_Texture* m_font_texture = nullptr;

			bool m_imguiStartNewFrame = true;

			friend class Renderer;
		};
	}

	/// Utility class for all other engine systems to call into.
	class IS_GRAPHICS Renderer
	{
	public:
		static Graphics::RHI_Buffer* CreateVertexBuffer(u64 sizeBytes, int stride, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateIndexBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateUniformBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateReadbackBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateStagingBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });
		static Graphics::RHI_Buffer* CreateRawBuffer(u64 sizeBytes, Graphics::RHI_Buffer_Overrides buffer_overrides = { });

		static void FreeVertexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeIndexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeUniformBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeReadbackBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeStagingBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeRawBuffer(Graphics::RHI_Buffer* buffer);

		static int GetVertexBufferCount();
		static int GetIndexBufferCount();
		static int GetUniformBufferCount();
		static int GetBufferCount(Graphics::BufferType bufferType);

		static Graphics::RHI_Texture* CreateTexture();
		static void FreeTexture(Graphics::RHI_Texture* texture);
		static bool HasTexture(Graphics::RHI_Texture* texture);

		static Graphics::GraphicsAPI GetGraphicsAPI();

		static Graphics::RHI_ResourceCache<Graphics::RHI_Buffer>* CreateBufferResourceCache();
		static Graphics::RHI_ResourceCache<Graphics::RHI_Texture>* CreateTextureResourceCache();

		static void FreeResourceCache(Graphics::IRHI_ResourceCache* resourceCache);

	private:
		static Graphics::RenderContext* s_context;
		friend class Graphics::RenderContext;
	};
}