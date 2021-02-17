#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/PixelFormat.h"
#include "Engine/Graphics/Enums.h"
#include <functional>
#include "glm/glm.hpp"

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

static const RenderGraphQueueFlags ComputeQueues = RENDER_GRAPH_QUEUE_COMPUTE_BIT |
RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT;

/// <summary>
/// Define an image attachment
/// </summary>
struct ImageAttachmentInfo
{
	PixelFormat Format = PixelFormat::Unknown;
	std::string Name = "";
	ImageUsageFlags ImageUsage = 0;
	U32 Width = 0;
	U32 Height = 0;
	U32 Depth = 1;
	U32 Samples = 1;
	U32 Levels = 1;
	U32 Layers = 1;
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

	enum {Unused = -1};

	RenderGraphResource(const Type& type, U32 index, const std::string& name)
		: m_type(type)
		, m_index(index)
		, m_name(name)
		, m_physicalIndex(Unused)
	{ }

	const Type& GetType() const { return m_type; }
	const U32& GetIndex() const { return m_index; }
	const U32& GetPhysicalIndex() const { return m_physicalIndex; }
	void SetPhysicalIndex(const U32& physicalIndex) { m_physicalIndex = physicalIndex; }

	void AddQueue(const RenderGraphQueueFlags& flags) { m_usedQueues |= flags; }
	void AddWrittenInPass(const U32& pass) { m_writtemInPasses.insert(pass); }
	void AddReadInPass(const U32& pass) { m_readInPasses.insert(pass); }

	const std::unordered_set<U32>& GetWritePasses() const { return m_writtemInPasses; }
	const std::unordered_set<U32>& GetReadPasses() const { return m_readInPasses; }

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
	ImageAttachmentInfo m_info = {};
	ImageUsageFlags m_usageFlags = 0;
	bool m_transient = false;
};

struct AccessedResouce
{
	PipelineStageFlags Stages = 0;
	AccessFlags Access = 0;
	ImageLayoutFlags Layout = 0;
};

struct AccessedTextureResource : AccessedResouce
{
	RenderTextureResouce* Texture = nullptr;
};

class IS_API RenderPass
{
public:
	explicit RenderPass(RenderGraph* graph, const U32& index, const std::string name,
		RenderGraphQueueFlags queue);
	~RenderPass();

	RenderGraph* GetGraph() const { return m_graph; }
	const U32& GetIndex() const { return m_index; }
	const U32& GetPhysicalPassIndex() const { return m_pyshicalIndex; }
	void SetPhysicalPassIndex(const U32& index) { m_pyshicalIndex = index; }

	const RenderGraphQueueFlags& GetQueue()const { return m_queue; }
	const std::string& GetPassName() const { return m_name; }

	RenderTextureResouce& AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment, const std::string& input = "");
	RenderTextureResouce& SetDepthStencilOutput(const std::string& name, ImageAttachmentInfo& attachment);
	RenderTextureResouce& AddAttachmentInput(const std::string& name);

	RenderTextureResouce& SetDepthStencilInput(const std::string& name);
	RenderTextureResouce& AddTextureInput(const std::string& name, PipelineStageFlags stages);

	void AddStorageInput(const std::string& name, BufferAttachmentInfo& attachment);
	void AddUniformInput(const std::string& name, BufferAttachmentInfo& attachment);

	const std::vector<RenderTextureResouce*>& GetColorInputs() const { return m_colorInputs; }
	const std::vector<RenderTextureResouce*>& GetColorOutputs() const { return m_colorOutputs; }
	const std::vector<RenderTextureResouce*>& GetAttachmentInputs() const { return m_attachmentsInputs; }
	const std::vector<RenderTextureResouce*>& GetHistoryInputs() const { return m_historyInputs; }
	const std::vector<AccessedTextureResource>& GetGenericTextureInputs() const { return m_genericTextures; }

	RenderTextureResouce* GetDepthStencilInput() const { return m_depthStencilInput; }
	RenderTextureResouce* GetDepthStencilOutput() const { return m_depthStencilOutput; }

	bool GetClearColor(U32 index, glm::vec4* value = nullptr);
	bool GetClearDepthStencil(glm::vec2* value = nullptr);

private:
	RenderGraph* m_graph;
	U32 m_index;
	U32 m_pyshicalIndex;
	RenderGraphQueueFlags m_queue;
	std::string m_name;
	std::vector<RenderTextureResouce*> m_colorInputs;
	std::vector<RenderTextureResouce*> m_colorOutputs;
	std::vector<RenderTextureResouce*> m_attachmentsInputs;
	std::vector<RenderTextureResouce*> m_historyInputs;
	std::vector<AccessedTextureResource> m_genericTextures;

	std::function<bool(U32, glm::vec4*)> m_clearColorFunc;
	std::function<bool(glm::vec2*)> m_clearDepthStencilFunc;

	RenderTextureResouce* m_depthStencilInput = nullptr;
	RenderTextureResouce* m_depthStencilOutput = nullptr;
};