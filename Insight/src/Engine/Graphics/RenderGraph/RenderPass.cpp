
#include "Engine/Graphics/RenderGraph/RenderPass.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/Module/WindowModule.h"

namespace Insight::Graphics
{
	ImageAttachmentInfo::ImageAttachmentInfo()
	{ }

	/// <summary>
	/// ImageAttachmentInfo::DepthAttachment
	/// </summary>
	/// <returns></returns>
	ImageAttachmentInfo ImageAttachmentInfo::DepthAttachment()
	{
		ImageAttachmentInfo info = { };
		info.Format = PixelFormat::D32_Float;
		info.Name = "DepthAttachment";
		return info;
	}


	/// <summary>
	/// RenderPass
	/// </summary>
	/// <param name="graph"></param>
	/// <param name="index"></param>
	/// <param name="name"></param>
	/// <param name="queue"></param>
	RenderPass::RenderPass(RenderGraph* graph, const u32& index, const std::string name,
						   RenderGraphQueueFlags queue)
		: m_graph(graph)
		, m_passIndex(index)
		, m_name(name)
		, m_queue(queue)
		, m_passQueue(RenderPassQueue::Default)
		, m_windowRect(Maths::Rect(0,0, (float)Module::WindowModule::GetWindow()->GetWidth(), (float)Module::WindowModule::GetWindow()->GetHeight()))
	{
	}

	RenderPass::~RenderPass()
	{
	}

	RenderGraphResource& RenderPass::AddOutput(const std::string& name, ImageAttachmentInfo& attachment, const ImageUsageFlags& usageFlags)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.SetPassName(m_name);
		res.AddQueue(m_queue);
		res.AddWrittenInPass(m_passIndex);
		res.TextureInfo.SetAttachmentInfo(attachment);
		res.TextureInfo.AddImageUsage(usageFlags);
		res.TextureInfo.ImageLayout = ImageLayout::Shader_Read_Only;
		m_colorOutputs.push_back(res.GetIndex());
		m_colorOutputHasher.Hash(name);

		if (attachment.Levels != 1)
		{
			res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src | (u32)ImageUsageFlagsBits::Transfer_Dst);
		}

		return res;
	}

	RenderGraphResource& RenderPass::AddInput(const std::string& name, const ImageUsageFlags& usageFlags)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.AddQueue(m_queue);
		res.AddReadInPass(m_passIndex);
		res.TextureInfo.AddImageUsage(usageFlags);
		m_colorInputs.push_back(res.GetIndex());
		return res;
	}

	RenderGraphResource& RenderPass::AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment)
	{
		auto& res = m_graph->GetTextureResouce(name);
		res.SetPassName(m_name);
		res.AddQueue(m_queue);
		res.AddWrittenInPass(m_passIndex);
		res.TextureInfo.SetAttachmentInfo(attachment);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Color_Attachment);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Sampled);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src);
		res.TextureInfo.ImageLayout = ImageLayout::Shader_Read_Only;
		m_colorOutputs.push_back(res.GetIndex());
		m_colorOutputHasher.Hash(name);

		if (attachment.Levels != 1)
		{
			res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src | (u32)ImageUsageFlagsBits::Transfer_Dst);
		}

		return res;
	}

	RenderGraphResource& RenderPass::AddColorOutput(const std::string& name)
	{
		auto& res = m_graph->GetTextureResouce(name);
		//ASSERT(res.TextureInfo.m_info.Width != 0 || res.TextureInfo.m_info.Height != 0 && "[RenderPass::AddColorOutput] Output color texture is not valid. Texture must have already been rendered to.");
		res.SetPassName(m_name);
		res.AddQueue(m_queue);
		res.AddWrittenInPass(m_passIndex);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Color_Attachment);
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src);
		res.TextureInfo.ImageLayout = ImageLayout::Shader_Read_Only;
		m_colorOutputs.push_back(res.GetIndex());
		m_colorOutputHasher.Hash(name);

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
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Sampled);
		//res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src);
		res.TextureInfo.ImageLayout = ImageLayout::Depth_Stencil_Read_Only;
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
		res.TextureInfo.AddImageUsage((u32)ImageUsageFlagsBits::Transfer_Src);
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

	RenderGraphResource& RenderPass::GetTextureResource(u32 index) const
	{
		return m_graph->GetTextureResouce(index);
	}

	RenderGraphResource& RenderPass::GetTextureResource(const std::string& name) const
	{
		return m_graph->GetTextureResouce(name);
	}

	GPUImage* RenderPass::GetPhysicalImage(u32 index) const
	{
		return m_graph->m_physicalImages.at(index);
	}

	GPUImageView* RenderPass::GetPhysicalImageView(u32 index) const
	{
		return m_graph->m_physicalImageViews.at(index);
	}
}