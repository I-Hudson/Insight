#pragma once
#include "RenderPass.h"
#include "glm/glm.hpp"
#include "Engine/Graphics/GPUDevice.h"

#define RENDER_PASS_ATTACHMENT_COUNT 6

namespace Insight::Graphics
{
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
			return (ImageUsage & (U32)ImageUsageFlagsBits::Storage) != 0;
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
	class GPUCmdBuffer;
	class PipelineEvent;

	struct RenderPassInfo
	{
		const Insight::Graphics::GPUImageView* ColorAttachments[RENDER_PASS_ATTACHMENT_COUNT];
		const Insight::Graphics::GPUImageView* DepthStenci = nullptr;
		U32 NumColorAttachments = 0;
		RenderPassOpFlags OpFlags = 0;
		U32 ClearAttachments = 0;
		U32 LoadAttachments = 0;
		U32 StoreAttachments = 0;
		U32 BaseLayer = 0;
		U32 NumLayers = 0;

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
			U32 ColorAttachments[RENDER_PASS_ATTACHMENT_COUNT];
			U32 InputAttachments[RENDER_PASS_ATTACHMENT_COUNT];
			U32 ResolveAttachments[RENDER_PASS_ATTACHMENT_COUNT];
			U32 NumColorAttachments = 0;
			U32 NumInputAttachments = 0;
			U32 NumResolveAttachments = 0;

			DepthStencil DepthStencilMode = DepthStencil::Read_Only;
		};

		// If 0/nullptr, assume a default subpass.
		const Subpass* subpass = nullptr;
		U32 NumSubpasses = 0;
	};

	/// <summary>
	/// Create a render graph to be used when rendering. This graph should describe 
	/// all passes needed inputs/output to each pass and the dependencies between 
	/// the single passes. 
	/// Currently the graph does not share resources between each pass. This needs to be 
	/// looked into to share resources instead of having unique resources per a pass.
	/// </summary>
	class IS_API RenderGraph
	{
	public:
		static RenderGraph* New();

		RenderGraph();
		virtual ~RenderGraph();

		RenderPass& AddPass(const std::string& name, RenderGraphQueueFlags queue);

		void SetSwapchainDimensions(const ResourceDimensions& dimensions) { m_swapchainDimensions = dimensions; }
		void SetbackBufferSource(const std::string& name);

		/// <summary>
		/// Build the graph and create any resources which are needed. This should 
		/// check for cached versions or almost everything.
		/// </summary>
		void Build();

		/// <summary>
		/// Execute the render graph. This should run through all the render passes and call
		/// their render function call backs. 
		/// </summary>
		void Execute();

		/// <summary>
		/// Reset the graph. This should just clear the RenderPasses.
		/// </summary>
		void Reset();

		/// <summary>
		/// Log the render graph to the console. For Debugging.
		/// </summary>
		void LogToConsole();

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

		void BuildPhysical();

	protected:
		bool m_built;
		bool m_changed;

		std::vector<RenderPass> m_passes;
		std::vector<RenderPass> m_passesCached;
		std::vector<RenderGraphResource> m_resources;

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
			U32 Index;
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
			// Framebuffer
			// Renderpass
			// Fence
			std::array<GPUCommandPool*, 32> CommandPools;
			std::array<GPUCommandBuffer*, 32> CommandBuffers;

			//Seampahores SwapchainAcquire;
			//Seampahores SwapchainRelease;

			struct DescriptorBinder
			{
				GPUDevice* Device;
				//GPUDescriptorPool DescriptorPool;
				u32 PoolSize = 256;

				//std::vector<DescriptorSet*> DescriptorWrites;
				//std::vector<DescriptorBufferInfo> BufferInfos;
				//std::vector<DescriptorImageInfo> ImageInfos;
			};
		};

		friend RenderPass;
	};
}