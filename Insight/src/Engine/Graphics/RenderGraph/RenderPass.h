#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/PixelFormat.h"
#include "Engine/Graphics/Enums.h"

class Texture;
class RenderGraph;

enum RenderGraphQueueFlagsBits
{
	RENDER_GRAPH_QUEUE_GRAPHICS_BIT			= 1 << 0,
	RENDER_GRAPH_QUEUE_COMPUTE_BIT			= 1 << 1,
	RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT	= 1 << 2,
	RENDER_GRAPH_QUEUE_ASYNC_GRAPHICS_BIT	= 1 << 3,
};
using RenderGraphQueueFlags = U32;

/// <summary>
/// Define an image attachment
/// </summary>
struct ImageAttachmentInfo
{
	PixelFormat Format;
	std::string Name;
	ImageUsageFlags ImageUsage;
	I32 Width = -1;
	I32 Height = -1;
	I32 Samples = -1;
	I32 Levels = -1;
	I32 Layers = -1;
	bool Persistent = true;
	bool UnormSRGBAlias = false;
	bool SupportsPrerotate = false;
};

struct BufferAttachmentInfo
{
	U64 Size;
	ShaderStage ShaderStages;
};

class RenderGraphResource
{
public:
	enum class Type
	{
		Buffer,
		Texture
	};

	RenderGraphResource(const Type& type, U32 index, const std::string& name)
		: m_type(type)
		, m_index(index)
		, m_name(name)
		, m_physicalIndex(-1)
	{ }

	const Type& GetType() const { return m_type; }
	void AddQueue(const RenderGraphQueueFlags& flags) { m_usedQueues |= flags; }
	void AddWrittenInPass(const U32& pass) { m_writtemInPasses.insert(pass); }
	void AddReadInPass(const U32& pass) { m_readInPasses.insert(pass); }

	const std::string& GetName() const { return m_name; }
	const RenderGraphQueueFlags& GetUsedQueues() const { return m_usedQueues; }

private:
	Type m_type;
	U32 m_index;
	U32 m_physicalIndex;
	std::unordered_set<U32> m_writtemInPasses;
	std::unordered_set<U32> m_readInPasses;
	std::string m_name;
	RenderGraphQueueFlags m_usedQueues = 0;
};

class RenderTextureResouce : public RenderGraphResource
{
public:
	RenderTextureResouce(const std::string& name, U32 index)
		: RenderGraphResource(Type::Texture, index, name)
	{ }

	const ImageAttachmentInfo& GetAttachmentInfo() const { return m_info; }
	void SetAttachmentInfo(const ImageAttachmentInfo& info) { m_info = info; }
	void AddImageUsage(const ImageUsageFlags& imageUsage) { m_usageFlags |= imageUsage; }
	const ImageUsageFlags& GetImageUsage() const { return m_usageFlags; }

	void SetTransientState(bool enable) { m_transient = enable; }
	const bool& GetTransientState() const { return m_transient; }

private:
	ImageAttachmentInfo m_info;
	ImageUsageFlags m_usageFlags;
	bool m_transient = false;
};

class IS_API RenderPass
{
public:
	explicit RenderPass(RenderGraph* graph, const U32& index, const std::string name,
		RenderGraphQueueFlags queue);
	~RenderPass();

	RenderGraph* GetGraph() const { return m_graph; }
	const U32& GetIndex() const { return m_index; }
	const RenderGraphQueueFlags& GetQueue()const { return m_queue; }
	const std::string& GetPassName() const { return m_name; }

	void AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment);
	void SetDepthStencilOutput(const std::string& name, ImageAttachmentInfo& attachment);

	void AddColorInput(const std::string& name);
	void AddTextureInput(const std::string& name, Texture* texture);

	void AddStorageInput(const std::string& name, BufferAttachmentInfo& attachment);
	void AddUniformInput(const std::string& name, BufferAttachmentInfo& attachment);

	const std::vector<RenderTextureResouce*>& GetColorInputs() { return m_colorInputs; }
	const std::vector<RenderTextureResouce*>& GetColorOutputs() { return m_colorOutputs; }

	const RenderTextureResouce* GetDepthStencilInput() const { return m_depthStencilInput; }
	const RenderTextureResouce* GetDepthStencilOutput() const { return m_depthStencilOutput; }

private:
	RenderGraph* m_graph;
	U32 m_index;
	U32 m_pyshicalIndex;
	RenderGraphQueueFlags m_queue;
	std::string m_name;
	std::vector<RenderTextureResouce*> m_colorInputs;
	std::vector<RenderTextureResouce*> m_colorOutputs;

	RenderTextureResouce* m_depthStencilInput = nullptr;
	RenderTextureResouce* m_depthStencilOutput = nullptr;
};