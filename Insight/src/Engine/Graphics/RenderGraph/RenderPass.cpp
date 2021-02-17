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

RenderTextureResouce& RenderPass::AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment, const std::string& input)
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

	if (!input.empty())
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.AddReadInPass(m_index);
		res.AddImageUsage((U32)ImageUsageFlagsBits::Color_Attachment);
		m_colorInputs.push_back(&res);
	}
	else
	{
		m_colorInputs.push_back(nullptr);
	}

	return res;
}

RenderTextureResouce& RenderPass::SetDepthStencilOutput(const std::string& name, ImageAttachmentInfo& attachment)
{
	auto& res = m_graph->GetTextureResouce(name);
	res.AddQueue(m_queue);
	res.AddWrittenInPass(m_index);
	res.SetAttachmentInfo(attachment);
	res.AddImageUsage((U32)ImageUsageFlagsBits::Depth_Stencil_Attachment);
	m_depthStencilOutput = &res;
	return res;
}

RenderTextureResouce& RenderPass::AddAttachmentInput(const std::string& name)
{
	auto& res = m_graph->GetTextureResouce(name);
	res.AddQueue(m_queue);
	res.AddReadInPass(m_index);
	res.AddImageUsage((U32)ImageUsageFlagsBits::Input_Attachment);
	m_attachmentsInputs.push_back(&res);
	return res;
}

RenderTextureResouce& RenderPass::AddTextureInput(const std::string& name, PipelineStageFlags stages)
{
	auto& res = m_graph->GetTextureResouce(name);
	res.AddQueue(m_queue);
	res.AddReadInPass(m_index);
	res.AddImageUsage((U32)ImageUsageFlagsBits::Sampled);

	auto itr = std::find_if(m_genericTextures.begin(), m_genericTextures.end(), [&](const AccessedTextureResource& acc)
		{
			return acc.Texture == &res;
		});

	if (itr != m_genericTextures.end())
	{
		return *itr->Texture;
	}

	AccessedTextureResource acc;
	acc.Texture = &res;
	acc.Layout = (U32)ImageLayout::Shader_Read_Only;
	acc.Access = (U32)Access::Shader_Read;

	if (stages != 0)
	{
		acc.Stages = stages;
	}
	else if ((m_queue & ComputeQueues) != 0)
	{
		acc.Stages = (U32)PipelineStage::Compute_Shader;
	}
	else
	{
		acc.Stages = (U32)PipelineStage::Fragment_Shader;
	}

	m_genericTextures.push_back(acc);
	return res;
}

RenderTextureResouce& RenderPass::SetDepthStencilInput(const std::string& name)
{
	auto& res = m_graph->GetTextureResouce(name);
	res.AddQueue(m_queue);
	res.AddReadInPass(m_index);
	res.AddImageUsage((U32)ImageUsageFlagsBits::Depth_Stencil_Attachment);
	m_depthStencilInput = &res;
	return res;
}

void RenderPass::AddStorageInput(const std::string& name, BufferAttachmentInfo& attachment)
{
}

void RenderPass::AddUniformInput(const std::string& name, BufferAttachmentInfo& attachment)
{
}

bool RenderPass::GetClearColor(U32 index, glm::vec4* value)
{
	if (m_clearColorFunc)
		return m_clearColorFunc(index, value);
	else
		return false;
}

bool RenderPass::GetClearDepthStencil(glm::vec2* value)
{
	if (m_clearDepthStencilFunc)
		return m_clearDepthStencilFunc(value);
	else
		return false;
}
