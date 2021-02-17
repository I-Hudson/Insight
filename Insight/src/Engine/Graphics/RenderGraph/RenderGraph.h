#pragma once
#include "RenderPass.h"
#include "glm/glm.hpp"
#include "Engine/Graphics/GPUDevice.h"

#define RENDER_PASS_ATTACHMENT_COUNT 6

struct ResourceDimensions
{
	PixelFormat Format = PixelFormat::Unknown;
	U32 Width = 0;
	U32 Height = 0;
	U32 Depth = 1;
	U32 Layers = 1;
	U32 Levels = 1;
	U32 Samples = 1;
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

class GPUImageView;
class GPUImage;
class GPUCmdBuffer;
class PipelineEvent;

struct RenderPassInfo
{
	const GPUImageView* ColorAttachments[RENDER_PASS_ATTACHMENT_COUNT];
	const GPUImageView* DepthStenci = nullptr;
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
	~RenderGraph();

	RenderPass& AddPass(const std::string& name, RenderGraphQueueFlags queue);

	RenderTextureResouce& GetTextureResouce(const std::string& name);

	void SetSwapchainDimensions(const ResourceDimensions& dimensions) { m_swapchainDimensions = dimensions; }
	void SetbackBufferSource(const std::string& name);

	/// <summary>
	/// Build the graph so it can be used for rendering.
	/// </summary>
	void Build();

	void SetupAttachments();
	virtual void EnqueueRenderPass() = 0;

	void Reset();
	void LogToConsole();

protected:

	ResourceDimensions GetResourceDimensions(const RenderTextureResouce& resource) const;

	/// <summary>
	/// Check that the graph is a-ok. All passes need to be check for layouts
	/// inputs and outputs. 
	/// </summary>
	void Validate();

	void TraverseDependencies(const RenderPass& pass, U32 stackCount);

	void DependPassesRecursive(const RenderPass& self, const std::unordered_set<U32>& writtenPasses,
		U32 stackCount, bool noCheck, bool ignoreSelf, bool mergeDependency);

	void FilterPasses(std::vector<U32>& list);

	/// <summary>
	/// Filter the passes into the best order. This also includes 
	/// </summary>
	void ReorderPasses(std::vector<U32>& passes);

	bool DependsOnPass(U32 dstPass, U32 srcPass);

	/// <summary>
	/// Figure out which physical resources we need. Here we will alias resources which can trivially alias via renaming.
	/// E.g. depth input -> depth output is just one physical attachment, similar with color.
	/// </summary>
	void BuildPhysicalResources();

	/// <summary>
	/// Try and merge passes together.
	/// </summary>
	void BuildPhysicalPasses();

	/// <summary>
	/// After merging physical passes and resources, if an image resource is only used in a single physical pass, make it transient.
	/// </summary>
	void BuildTransients();

	/// <summary>
	/// Create our renderpasses.
	/// </summary>
	void BuildRenderPassInfo();

	/// <summary>
	/// For each render pass in isolation, figure out the barriers required.
	/// </summary>
	void BuildBarriers();

	void BuildPhysicalBarriers();

	/// <summary>
	/// Figure out which images can alias with each other.
	/// Also build virtual "transfer" barriers. These things only copy events over to other physical resources.
	/// </summary>
	void BuidAliases();

	void SetupPhysicalBuffer(GPUDevice& device, U32 index);
	void SetupPhysicalImage(GPUDevice& device, U32 index);

protected:
	bool m_built;
	bool m_changed;

	std::vector<RenderPass*> m_passes;
	std::vector<RenderGraphResource*> m_resources;

	std::unordered_map<std::string, U32> m_passToIndex;
	std::unordered_map<std::string, U32> m_resourceToIndex;

	std::vector<U32> m_passStack;
	std::vector<std::unordered_set<U32>> m_passDependencies;
	std::vector<std::unordered_set<U32>> m_passMergeDependencies;

	struct PhysicalResources
	{
		std::vector<ResourceDimensions> Dimensions;
		std::vector<bool> PhysicalImageHasHistory;

		std::vector<GPUImageView*> Attachments;
		std::vector<GPUCmdBuffer*> Buffers;
		std::vector<GPUImage*> ImageAttachments;
		std::vector<GPUImage*> HistoryImageAttachments;
		std::vector<U32> Aliases;

		std::vector<PipelineEvent*> Events;
		std::vector<PipelineEvent*> HistoryEvents;
	};
	PhysicalResources m_physicalResources;

	ResourceDimensions m_swapchainDimensions;
	U32 m_swapchainPhysicalIndex = RenderGraphResource::Unused;
	std::string m_backBufferSource;

	struct Barrier
	{
		U32 ResourceIndex;
		ImageLayout Layout;
		AccessFlags Access;
		PipelineStageFlags Stages;
		bool History;
	};

	struct Barriers
	{
		std::vector<RenderGraph::Barrier> Invalidate;
		std::vector<RenderGraph::Barrier> Flush;
	};

	std::vector<Barriers> m_passBarriers;

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
		U32 Target;
		U32 PhysicalIndex;
	};

	struct MipmapRequest
	{
		U32 PhysicalResource;
		PipelineStageFlags Stages;
		AccessFlags Access;
		ImageLayout Layout;
	};

	struct PhysicalPass
	{
		std::vector<U32> Passes;
		std::vector<U32> Discards;
		std::vector<RenderGraph::Barrier> Invalidate;
		std::vector<RenderGraph::Barrier> Flush;
		std::vector<RenderGraph::Barrier> History;
		std::vector<std::pair<U32, U32>> AliasTransfer;

		RenderPassInfo RenderPassInfo;
		std::vector<RenderPassInfo::Subpass> Subpasses;
		std::vector<U32> PhysicalColorAttachments;
		U32 PhysicalDepthStencilAttachment = RenderGraphResource::Unused;
		
		std::vector<RenderGraph::ColorClearRequest> ColorClearRequests;
		RenderGraph::DepthClearRequest DepthClearRequest;
		std::vector<std::vector<RenderGraph::ScaledClearRequest>> ScaledClearRequests;
		std::vector<RenderGraph::MipmapRequest> MipmapRequests;

		U32 Layers = 1;
	};
	std::vector<PhysicalPass> m_physicalPasses;
};