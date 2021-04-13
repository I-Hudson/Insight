#include "ispch.h"
#include "Engine/Graphics/RenderGraph/RenderPass.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"

namespace Insight::Graphics
{
	RenderPass::RenderPass(RenderGraph* graph, const u32& index, const std::string name,
						   RenderGraphQueueFlags queue)
		: m_graph(graph)
		, m_passIndex(index)
		, m_name(name)
		, m_queue(queue)
	{
	}

	RenderPass::~RenderPass()
	{
	}

	RenderGraphResource& RenderPass::AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.SetPassName(m_name);
		res.AddQueue(m_queue);
		res.AddWrittenInPass(m_passIndex);
		res.TextureInfo.SetAttachmentInfo(attachment);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Color_Attachment);
		res.TextureInfo.ImageLayout = ImageLayout::Color_Attachment;
		m_colorOutputs.push_back(res.GetIndex());

		if (attachment.Levels != 1)
		{
			res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src | (u32)ImageUsageFlagsBits::Transfer_Dst);
		}

		return res;
	}

	RenderGraphResource& RenderPass::AddColorInput(const std::string& name)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.AddQueue(m_queue);
		res.AddReadInPass(m_passIndex);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Sampled);
		m_colorInputs.push_back(res.GetIndex());
		return res;
	}

	RenderGraphResource& RenderPass::SetDepthStencilOutput(const std::string& name, ImageAttachmentInfo& attachment)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.SetPassName(m_name);
		res.AddQueue(m_queue);
		res.AddWrittenInPass(m_passIndex);
		res.TextureInfo.SetAttachmentInfo(attachment);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Depth_Stencil_Attachment);
		res.TextureInfo.ImageLayout = ImageLayout::Depth_Stencil_Attachment;
		m_depthStencilOutput = res.GetIndex();
		return res;
	}

	RenderGraphResource& RenderPass::AddAttachmentInput(const std::string& name)
	{
		ASSERT(false && "[RenderPass::AddAttachmentInput] Not supported.");
		auto& res = m_graph->GetTextureResouce(name);
		res.AddQueue(m_queue);
		res.AddReadInPass(m_passIndex);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Input_Attachment);
		res.TextureInfo.ImageLayout = ImageLayout::Color_Attachment;
		m_attachmentsInputs.push_back(res.GetIndex());
		return res;
	}

	/*RenderGraphResource& RenderPass::AddTextureInput(const std::string& name, PipelineStageFlags stages)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.AddQueue(m_queue);
		res.AddReadInPass(m_passIndex);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Sampled);

		auto itr = std::find_if(m_genericTextures.begin(), m_genericTextures.end(), [&](const RenderGraphResource& resource)
		{
			return resource.GetIndex() == res.GetIndex();
		});

		if (itr != m_genericTextures.end())
		{
			return *itr;
		}

		AccessedResouce acc;
		acc.Layout = (u32)ImageLayout::Shader_Read_Only;
		acc.Access = (u32)Access::Shader_Read;

		if (stages != 0)
		{
			acc.Stages = stages;
		}
		else if ((m_queue & ComputeQueues) != 0)
		{
			acc.Stages = (u32)PipelineStage::Compute_Shader;
		}
		else
		{
			acc.Stages = (u32)PipelineStage::Fragment_Shader;
		}
		res.AccessedResource = acc;

		m_genericTextures.push_back(res);
		return res;
	}*/

	RenderGraphResource& RenderPass::SetDepthStencilInput(const std::string& name)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.AddQueue(m_queue);
		res.AddReadInPass(m_passIndex);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Depth_Stencil_Attachment);
		m_depthStencilInput = res.GetIndex();
		return res;
	}

	//void RenderPass::AddStorageInput(const std::string& name, BufferAttachmentInfo& attachment)
	//{
	//}

	//RenderGraphResource& RenderPass::AddUniformInput(const std::string& name, BufferAttachmentInfo& attachment)
	//{
	//	auto& res = m_graph->GetBufferResource(name);
	//	res.AddQueue(m_queue);
	//	res.AddReadInPass(m_passIndex);
	//	res.BufferInfo.Size = attachment.Size;
	//	res.BufferInfo.Usage = GPUBufferFlags::TRANSFER_SRC | GPUBufferFlags::UNIFORM;

	//	auto itr = std::find_if(m_uniformBuffer.begin(), m_uniformBuffer.end(), [res](const RenderGraphResource& resource)
	//	{
	//		return resource.GetIndex() == res.GetIndex();
	//	});

	//	AccessedResouce acc;
	//	acc.Layout = (u32)ImageLayout::General;
	//	acc.Access = (u32)Access::Uniform_Read;
	//	if (attachment.ShaderStages != 0)
	//	{
	//		acc.Stages = attachment.ShaderStages;
	//	}
	//	m_uniformBuffer.push_back(res);
	//	return m_uniformBuffer.back();
	//}

	RenderGraphResource& RenderPass::GetDepthStencilInput() const
	{
		return m_graph->GetTextureResouce(m_depthStencilInput);
	}

	RenderGraphResource& RenderPass::GetDepthStencilOutput() const
	{
		return m_graph->GetTextureResouce(m_depthStencilOutput);
	}
}