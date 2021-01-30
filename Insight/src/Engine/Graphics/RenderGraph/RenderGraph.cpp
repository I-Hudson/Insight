#include "ispch.h"
#include "RenderGraph.h"

RenderGraph::RenderGraph()
	: m_built(false)
	, m_changed(false)
{

}

RenderGraph::~RenderGraph()
{
}

RenderPass& RenderGraph::AddPass(const std::string& name, RenderGraphQueueFlags queue)
{
	auto it = m_passToIndex.find(name);
	if (it != m_passToIndex.end())
	{
		return *m_passes[it->second];
	}
	else
	{
		U32 newIndex = m_passes.size();
		m_passes.emplace_back(new RenderPass(this, newIndex, name, queue));
		m_passToIndex[name] = newIndex;
		return *m_passes.back();
	}
}

RenderTextureResouce& RenderGraph::GetTextureResouce(const std::string& name)
{
	auto it = m_resourceToIndex.find(name);
	if (it != m_resourceToIndex.end())
	{
		ASSERT(m_resources[it->second]->GetType() == RenderGraphResource::Type::Texture);
		return static_cast<RenderTextureResouce&>(*m_resources[it->second]);
	}
	else
	{
		U32 newIndex = m_resources.size();
		m_resources.emplace_back(new RenderTextureResouce(name, newIndex));
		m_resourceToIndex[name] = newIndex;
		return static_cast<RenderTextureResouce&>(*m_resources.back());
	}
}

void RenderGraph::Build()
{
	Validate();
}

void RenderGraph::Execute()
{
}

void RenderGraph::PrintToConsole()
{
}

ResourceDimensions RenderGraph::GetResourceDimensions(const RenderTextureResouce& resource) const
{
	ResourceDimensions dim;
	auto& info = resource.GetAttachmentInfo();
	dim.Layers = info.Layers;
	dim.Samples = info.Samples;
	dim.Format = info.Format;
	dim.Transient = resource.GetTransientState();
	dim.Persistent = info.Persistent;
	dim.UnormSRGB = info.UnormSRGBAlias;
	dim.Queues = resource.GetUsedQueues();
	dim.ImageUsage = info.ImageUsage | resource.GetImageUsage();
	dim.Name = resource.GetName();

	dim.Width = m_swapchainDimensions.Width;
	dim.Height = m_swapchainDimensions.Height;
	dim.Depth = m_swapchainDimensions.Depth;

	if (dim.Format == PixelFormat::Unknown)
	{
		dim.Format = m_swapchainDimensions.Format;
	}

	const auto numLevels = [](unsigned width, unsigned height, unsigned depth) -> unsigned {
		unsigned levels = 0;
		unsigned max_dim = std::max(std::max(width, height), depth);
		while (max_dim)
		{
			levels++;
			max_dim >>= 1;
		}
		return levels;
	};

	dim.Levels = std::min(numLevels(dim.Width, dim.Height, dim.Depth), info.Levels == 0 ? ~0u : info.Levels);
	return dim;
}

void RenderGraph::Validate()
{
	for (auto& passPtr : m_passes)
	{
		auto& pass = passPtr;
		if (pass->GetColorInputs().size() != pass->GetColorOutputs().size())
		{
			throw std::logic_error("Size of color inputs must match color outputs.");
		}

		U32 numInputs = pass->GetColorInputs().size();
		for (U32 i = 0; i < numInputs; ++i)
		{
			if (pass->GetColorInputs()[i])
			{
				continue;
			}
		}

		if (pass->GetDepthStencilInput() && pass->GetDepthStencilOutput())
		{
			if (GetResourceDimensions(*pass->GetDepthStencilInput()) !=
				GetResourceDimensions(*pass->GetDepthStencilOutput()))
			{
				throw std::logic_error("Depth stencil dimensions mismatch.");
			}
		}
	}
}

void RenderGraph::Preprocess()
{
}

void RenderGraph::MergePasses(RenderPass& pass1, RenderPass& pass2)
{
}
