#include "ispch.h"
#include "RenderPass.h"
#include "RenderGraph.h"

RenderPass::RenderPass(RenderGraph* graph, const U32& index, const std::string name,
	RenderGraphQueueFlags queue)
	: m_graph(graph)
	, m_index(index)
	, m_name(name)
	, m_queue(queue)
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment)
{
	auto& res = m_graph->GetTextureResouce(name);
	res.AddQueue(m_queue);
	res.AddWrittenInPass(m_index);
	res.SetAttachmentInfo(attachment);
	res.AddImageUsage((U32)ImageUsageFlagsBits::Color_Attachment);
	m_colorOutputs.push_back(&res);

	if (attachment.Levels != 1)
	{
		res.AddImageUsage((U32)ImageUsageFlagsBits::Transfer_Src | (U32)ImageUsageFlagsBits::Transfer_Dst);
	}

	m_colorInputs.push_back(nullptr);
}

void RenderPass::SetDepthStencilOutput(const std::string& name, ImageAttachmentInfo& attachment)
{
}

void RenderPass::AddColorInput(const std::string& name)
{
}

void RenderPass::AddTextureInput(const std::string& name, Texture* texture)
{
}

void RenderPass::AddStorageInput(const std::string& name, BufferAttachmentInfo& attachment)
{
}

void RenderPass::AddUniformInput(const std::string& name, BufferAttachmentInfo& attachment)
{
}
