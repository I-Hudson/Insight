#pragma once
#include "RenderPass.h"

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
	RenderGraph();
	~RenderGraph();

	RenderPass& AddPass(const std::string& name, RenderGraphQueueFlags queue);

	RenderTextureResouce& GetTextureResouce(const std::string& name);

	void SetSwapchainDimensions(const ResourceDimensions& dimensions) { m_swapchainDimensions = dimensions; }

	/// <summary>
	/// Build the graph so it can be used for rendering.
	/// </summary>
	void Build();

	void Execute();
	void Reset();

#ifdef IS_DEBUG
	void PrintToConsole();
#endif

private:

	ResourceDimensions GetResourceDimensions(const RenderTextureResouce& resource) const;

	/// <summary>
	/// Check that the graph is a-ok. All passes need to be check for layouts
	/// inputs and outputs. 
	/// </summary>
	void Validate();

	/// <summary>
	/// Filter the passes into the best order. This also includes 
	/// </summary>
	void Preprocess();

	/// <summary>
	/// We have found two passes that can be merges into a single pass. This should 
	/// be done using subpasses.
	/// </summary>
	/// <param name="pass1"></param>
	/// <param name="pass2"></param>
	void MergePasses(RenderPass& pass1, RenderPass& pass2);

private:
	bool m_built;
	bool m_changed;

	std::vector<RenderPass*> m_passes;
	std::vector<RenderGraphResource*> m_resources;

	std::unordered_map<std::string, U32> m_passToIndex;
	std::unordered_map<std::string, U32> m_resourceToIndex;

	std::vector<U32> m_passStack;

	ResourceDimensions m_swapchainDimensions;
};