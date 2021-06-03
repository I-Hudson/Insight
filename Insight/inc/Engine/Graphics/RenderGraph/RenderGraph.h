#pragma once
#include "RenderPass.h"
#include "glm/glm.hpp"
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Graphics/GPUDescriptorSet.h"
#include "Engine/Graphics/GPUSync.h"
#include "Engine/Graphics/GPUSwapchain.h"

#define RENDER_PASS_ATTACHMENT_COUNT 6

namespace Insight::Graphics
{
	class GPUCmdBuffer;
	class PipelineEvent;
	class GPUImage;
	class GPUSwapchain;

	using RGOnGraphBuiltFunc = std::function<void()>;

	struct ResourceDimensions
	{
		PixelFormat Format = PixelFormat::Unknown;
		u32 Width = 0;
		u32 Height = 0;
		u32 Depth = 1;
		u32 Layers = 1;
		u32 Levels = 1;
		u32 Samples = 1;
		bool Transient = false;
		bool UnormSRGB = false;
		bool Persistent = true;
		RenderGraphQueueFlags Queues = 0;
		ImageUsageFlags ImageUsage = 0;

		bool operator==(const ResourceDimensions& other) const
		{
			return Format == other.Format &&
				Width == other.Width &&
				Height == other.Height &&
				Depth == other.Depth &&
				Layers == other.Layers &&
				Levels == other.Levels &&
				Transient == other.Transient &&
				Persistent == other.Persistent &&
				UnormSRGB == other.UnormSRGB;
			// image_usage is deliberately not part of this test.
			// queues is deliberately not part of this test.
		}

		bool operator!=(const ResourceDimensions& other) const
		{
			return !(*this == other);
		}

		bool UsesSemaphore() const
		{
			// If more than one queue is used for a resource, we need to use semaphores.
			auto physical_queues = Queues;

			// Regular compute uses regular graphics queue.
			if (physical_queues & RENDER_GRAPH_QUEUE_COMPUTE_BIT)
				physical_queues |= RENDER_GRAPH_QUEUE_GRAPHICS_BIT;
			physical_queues &= ~RENDER_GRAPH_QUEUE_COMPUTE_BIT;
			return (physical_queues & (physical_queues - 1)) != 0;
		}

		bool IsStorageImage() const
		{
			return (ImageUsage & (u32)ImageUsageFlagsBits::Storage) != 0;
		}

		bool IsBufferLike() const
		{
			return IsStorageImage();
		}

		std::string Name;
	};

	namespace Insight::Graphics
	{
		class GPUImageView;
		class GPUImage;
	}

	struct RenderPassInfo
	{
		const Insight::Graphics::GPUImageView* ColorAttachments[RENDER_PASS_ATTACHMENT_COUNT];
		const Insight::Graphics::GPUImageView* DepthStenci = nullptr;
		u32 NumColorAttachments = 0;
		RenderPassOpFlags OpFlags = 0;
		u32 ClearAttachments = 0;
		u32 LoadAttachments = 0;
		u32 StoreAttachments = 0;
		u32 BaseLayer = 0;
		u32 NumLayers = 0;

		glm::vec4 ClearColor[RENDER_PASS_ATTACHMENT_COUNT];
		glm::vec2 ClearDepthStencil = { 1.0f, 0.0f };

		enum class DepthStencil
		{
			None,
			Read_Only,
			Read_Write
		};

		struct Subpass
		{
			u32 ColorAttachments[RENDER_PASS_ATTACHMENT_COUNT];
			u32 InputAttachments[RENDER_PASS_ATTACHMENT_COUNT];
			u32 ResolveAttachments[RENDER_PASS_ATTACHMENT_COUNT];
			u32 NumColorAttachments = 0;
			u32 NumInputAttachments = 0;
			u32 NumResolveAttachments = 0;

			DepthStencil DepthStencilMode = DepthStencil::Read_Only;
		};

		// If 0/nullptr, assume a default subpass.
		const Subpass* subpass = nullptr;
		u32 NumSubpasses = 0;
	};

	class GPURenderGraphPass
	{
	public:
		GPURenderGraphPass() { }
		virtual ~GPURenderGraphPass() { }

		static GPURenderGraphPass* New();

		virtual void Init(RenderPass& renderPass) = 0;

		RenderPass& GetRenderPass() const;
		bool IsSwapchainPass() const { return m_swapchainPass; }

	private:
		virtual void InitForSwapchain(GPUSwapchain* swapchain, GPUImage* image) = 0;


	protected:
		std::vector<const GPUImage*> m_colourAttachments;
		u32 m_colorAttachmentsCount;
		const GPUImageView* m_depthStenci = nullptr;
		bool m_swapchainPass = false;

		// Wait Fence and Semaphore.
		u32 m_renderPassIndex = -1;
		RenderGraph* m_graph;

		friend RenderGraph;
	};

	/// <summary>
	/// Create a render graph to be used when rendering. This graph should describe 
	/// all passes needed inputs/output to each pass and the dependencies between 
	/// the single passes. 
	/// Currently the graph does not share resources between each pass. This needs to be 
	/// looked into to share resources instead of having unique resources per a pass.
	/// </summary>
	class IS_API RenderGraph : public Core::TSingleton<RenderGraph>
	{
	public:
		static RenderGraph* New();

		RenderGraph();
		virtual ~RenderGraph();

		RenderPass& AddPass(const std::string& name, RenderGraphQueueFlags queue);
		RenderPass& GetPass(const std::string& name);
		bool HasResource(const std::string& name);

		void SetbackBufferSource(const std::string& name);

		void SetSwapchainDimensions(const ResourceDimensions& dimensions) { m_swapchainDimensions = dimensions; }
		const std::string& GetBackbufferSourceName() const { return m_backBufferSource; }
		RenderGraphResource& GetBackbufferSource() { return m_resources.at(m_resourceToIndex.at(m_backBufferSource)); }
		const u32& GetFrameIndex() const { return m_frameIndex; }

		GPUImage* GetPhysicalImage(std::string name);
		GPUImage* GetPhysicalImage(u32 index) { return m_physicalImages.at(index); }
		GPUImageView* GetPhysicalImageView(u32 index) { return m_physicalImageViews.at(index); }

		/// <summary>
		/// Build the graph and create any resources which are needed. This should 
		/// check for cached versions or almost everything.
		/// </summary>
		void Build();

		/// <summary>
		/// Execute the render graph. This should run through all the render passes and call
		/// their render function call backs. 
		/// </summary>
		void Execute(RenderList* renderList);

		/// <summary>
		/// Reset the graph. This should just clear the RenderPasses.
		/// </summary>
		void Reset();

		/// <summary>
		/// Log the render graph to the console. For Debugging.
		/// </summary>
		void LogToConsole();

		void RegisterOnGraphBuiltFunc(std::string name, RGOnGraphBuiltFunc func);
		void UnregisterOnGraphBuiltFunc(std::string name);

	protected:
		ResourceDimensions GetResourceDimensions(const RenderGraphResource& resource) const;
		RenderGraphResource& GetTextureResouce(const std::string& name);
		RenderGraphResource& GetTextureResouce(const u32& index);
		//RenderGraphResource& GetBufferResource(const std::string& name);

		/// <summary>
		/// TODO: Validate each Render Pass.
		/// </summary>
		/// <returns></returns>
		bool ValidateRenderPasses();

		void GetPassDependencies();

		/// <summary>
		/// TODO: Go through all the Render Pass and reorder them if needed. This would allow passes to be added 
		/// at any stage.
		/// </summary>
		void ReorderRenderPasses(std::vector<u32>& flattenedPasses);

		void FilterPasses(std::vector<u32>& passList);

		bool DependsOnPass(u32 dstPass, u32 srcPass);

		void TraversePassDependenices(RenderPass& pass, u32 stackCount);

		void ResourceDependPass(RenderPass& pass, const std::unordered_set<u32>& writtenPasses, u32 stackCount, bool noCheck, bool ignoreSelf);

		/// <summary>
		/// Create all the physical/device resources we need. Example: images and image views.
		/// </summary>
		void BuildPhysical();

		/// <summary>
		/// Rebuild the swapchain as it is not longer optimal.
		/// </summary>
		void SwapchainRebuild();

	protected:
		bool m_built;
		bool m_changed;

		std::vector<RenderPass> m_passes;
		std::vector<RenderPass> m_passesCached;
		std::vector<RenderGraphResource> m_resources;
		RenderPass m_swapchainPresentPass;

		std::vector<GPUImage*> m_physicalImages;
		std::vector<GPUImageView*> m_physicalImageViews;
		//std::vector<GPUPipeline> m_pipelineCache;

		std::unordered_map<std::string, u32> m_passToIndex;
		std::unordered_map<std::string, u32> m_resourceToIndex;

		ResourceDimensions m_swapchainDimensions;
		u32 m_swapchainPhysicalIndex = RenderGraphResource::Unused;
		std::string m_backBufferSource;

		std::vector<u32> m_passStack;

		struct ColorClearRequest
		{
			RenderPass* Pass;
			glm::vec4* Target;
			u32 Index;
		};

		struct DepthClearRequest
		{
			RenderPass* Pass;
			glm::vec2* Target;
		};

		struct ScaledClearRequest
		{
			u32 Target;
			u32 PhysicalIndex;
		};

		struct MipmapRequest
		{
			u32 PhysicalResource;
			PipelineStageFlags Stages;
			AccessFlags Access;
			ImageLayout Layout;
		};

		struct FrameSubmision
		{
			GPUCommandPool* CommandPools;
			GPUCommandBuffer* CommandBuffers;

			GPUFence* Fence;
			GPUSemaphore* SwapchainImageAquired;
			GPUSemaphore* Present;

			GPUDescriptorAllocator* DescriptorAllocator;
			GPUDescriptorLayoutCache* DescriptorLayoutCache;
			/// <summary>
			/// Handle all the descriptors within this frame.
			/// </summary>
			GPUDescriptorBuilder* DescriptorBuilder;

			/// <summary>
			/// Store the render passes as FrameBuffer* pointers. 
			/// </summary>
			std::unordered_map<u64, GPURenderGraphPass*> RenderPasses;
			std::vector<RenderPass> Passes;
			std::vector<u32> PassStack;
			RenderList* RenderList;

			FrameBufferResources Buffers;

			bool Initialised = false;

			void Init();
			void Reset();
			void ReleaseGPU();
		};

		struct SwapchainSubmision
		{
			GPUShader* Shader;
			GPUPipeline* Pipeline;
			GPUImage* Image;
			GPURenderGraphPass* GraphPass;
		};
		std::vector<SwapchainSubmision> m_swapchainSubmision;

		public:
			const FrameSubmision& GetFrame(u32 index) const { return m_frames.at(index); }
			const FrameSubmision& GetCurrentFrame() const { return m_frames.at(m_frameIndex); }
			const u32& GetCurrentFrameIndex() const { return m_frameIndex; }
			const SwapchainSubmision& GetCurrentSwapchainSubmision() const { return m_swapchainSubmision.at(m_swapchainImageIndex); }
			const u32& GetFrameCount() const { return c_MaxFrameCount; }
		private:

		std::vector<FrameSubmision> m_frames;
		u32 m_frameIndex;
		u32 m_swapchainImageIndex = 0;
		const u32 c_MaxFrameCount = 3;
		glm::ivec2 m_renderGraphRenderSize;

		std::unordered_map<std::string, RGOnGraphBuiltFunc> m_onGraphBuiltFuncs;
		std::unordered_map<std::string, std::function<void()>> m_queuedFuncs;
		GPUSwapchain* m_swapchain;

		friend GPURenderGraphPass;
		friend RenderPass;
	};
}