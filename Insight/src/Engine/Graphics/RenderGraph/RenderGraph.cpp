#include "ispch.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include "Engine/Config/Config.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Graphics/PixelFormatExtensions.h"
#include "Engine/Graphics/Image/GPUImage.h"

namespace Insight::Graphics
{
	RenderGraph* RenderGraph::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			//case GraphicsRendererAPI::Vulkan: return ::New<RenderGraphVulkan>();
		}
		return ::New<RenderGraph>();
		ASSERT(false && "[RenderGraph::New] API implementation is missing.")
			return nullptr;
	}

	RenderGraph::RenderGraph()
		: m_built(false)
		, m_changed(false)
	{

	}

	RenderGraph::~RenderGraph()
	{
		for (auto* image : m_physicalImages)
		{
			::Delete(image);
		}
		for (auto* imageView : m_physicalImageViews)
		{
			::Delete(imageView);
		}
	}

	RenderPass& RenderGraph::AddPass(const std::string& name, RenderGraphQueueFlags queue)
	{
		auto it = m_passToIndex.find(name);
		if (it != m_passToIndex.end())
		{
			return m_passes.at(it->second);
		}
		else
		{
			U32 newIndex = (U32)m_passes.size();
			m_passes.push_back(RenderPass(this, newIndex, name, queue));
			m_passToIndex[name] = newIndex;
			return m_passes.back();
		}
	}

	void RenderGraph::SetbackBufferSource(const std::string& name)
	{
		m_backBufferSource = name;
	}

	void RenderGraph::Build()
	{
		auto backbufferItr = m_resourceToIndex.find(m_backBufferSource);
		if (backbufferItr == m_resourceToIndex.end())
		{
			IS_ERROR("[RenderGraph::Build] There is no back buffer resource.");
			return;
		}

		if (!ValidateRenderPasses())
		{
			IS_ERROR("[RenderGraph::Build] RenderGraph was unable to be validated.");
			return;
		}

		GetPassDependencies();
		ReorderRenderPasses(m_passStack);

		BuildPhysical();
	}

	void RenderGraph::Execute()
	{
	}

	void RenderGraph::Reset()
	{
		m_passes.clear();
		m_passToIndex.clear();
		m_passStack.clear();
	}

	void RenderGraph::LogToConsole()
	{
	}

	RenderGraphResource& RenderGraph::GetTextureResouce(const std::string& name)
	{
		auto it = m_resourceToIndex.find(name);
		if (it != m_resourceToIndex.end())
		{
			return GetTextureResouce(it->second);
		}
		else
		{
			U32 newIndex = (U32)m_resources.size();
			m_resources.push_back(RenderGraphResource(RenderGraphResource::Type::Texture, newIndex, name));
			m_resourceToIndex[name] = newIndex;
			return m_resources.back();
		}
	}

	RenderGraphResource& RenderGraph::GetTextureResouce(const u32& index)
	{
		ASSERT(m_resources.at(index).GetType() == RenderGraphResource::Type::Texture);
		return m_resources.at(index);
	}

	//RenderGraphResource& RenderGraph::GetBufferResource(const std::string& name)
	//{
	//	auto it = m_resourceToIndex.find(name);
	//	if (it != m_resourceToIndex.end())
	//	{
	//		ASSERT(m_resources[it->second].GetType() == RenderGraphResource::Type::Buffer);
	//		return m_resources.at(it->second);
	//	}
	//	else
	//	{
	//		U32 newIndex = (U32)m_resources.size();
	//		m_resources.push_back(RenderGraphResource(RenderGraphResource::Type::Buffer, newIndex, name));
	//		m_resourceToIndex[name] = newIndex;
	//		return m_resources.back();
	//	}
	//}

	bool RenderGraph::ValidateRenderPasses()
	{
		return true;
	}

	void RenderGraph::GetPassDependencies()
	{
		u32 backbufferResourceIndex = m_resourceToIndex[m_backBufferSource];
		auto& backbufferResource = m_resources[backbufferResourceIndex];
		ASSERT(!backbufferResource.GetWritePasses().empty() && "[RenderGraph::ReorderRenderPasses] Back buffer resource is not written to.");

		m_passStack.reserve(backbufferResource.GetWritePasses().size());
		for (auto& pass : backbufferResource.GetWritePasses())
		{
			m_passStack.push_back(pass);
		}

		std::vector<u32> tmpPassStack = m_passStack;
		for (auto& pushedPass : tmpPassStack)
		{
			auto& pass = m_passes[pushedPass];
			TraversePassDependenices(pass, 0);
		}

		std::reverse(m_passStack.begin(), m_passStack.end());
		FilterPasses(m_passStack);
	}

	void RenderGraph::ReorderRenderPasses(std::vector<u32>& flattenedPasses)
	{
		// We now have all the passes and which other dependent passes.
		std::vector<u32> unorderedPasses;
		unorderedPasses.reserve(flattenedPasses.size());
		std::swap(flattenedPasses, unorderedPasses);

		const auto schedule = [&](unsigned index) {
			// Need to preserve the order of remaining elements.
			flattenedPasses.push_back(unorderedPasses[index]);
			std::move(unorderedPasses.begin() + index + 1,
				 unorderedPasses.end(),
				 unorderedPasses.begin() + index);
			unorderedPasses.pop_back();
		};

		schedule(0);
		while (!unorderedPasses.empty())
		{
			u32 bestOverlap = 0;
			u32 bestCandidate = 0;

			for (size_t i = 0; i < unorderedPasses.size(); ++i)
			{
				u32 overlapFactor = 0;
				for (auto itr = flattenedPasses.rbegin(); itr != flattenedPasses.rend(); ++itr)
				{
					if (DependsOnPass(unorderedPasses[i], *itr))
					{
						break;
					}
					++overlapFactor;
				}

				if (overlapFactor >= bestOverlap)
				{
					continue;
				}

				bool possibleCandidate = true;
				for (u32 j = 0; j < i; ++j)
				{
					auto& pass = m_passes.at(unorderedPasses.at(i));
					auto& pass2 = m_passes.at(unorderedPasses.at(j));
					if (pass.GetDependentPasses().find(pass2.GetPassIndex()) != pass.GetDependentPasses().end())
					{
						possibleCandidate = false;
						break;
					}
				}

				if (!possibleCandidate)
				{
					continue;
				}

				bestCandidate = i;
				bestOverlap = overlapFactor;
			}
			schedule(bestCandidate);
		}
	}

	void RenderGraph::FilterPasses(std::vector<u32>& passList)
	{
		std::unordered_set<u32> seen;

		auto output_itr = passList.begin();
		for (auto itr = passList.begin(); itr != passList.end(); ++itr)
		{
			if (!seen.count(*itr))
			{
				*output_itr = *itr;
				seen.insert(*itr);
				++output_itr;
			}
		}
		passList.erase(output_itr, passList.end());
	}

	bool RenderGraph::DependsOnPass(u32 dstPass, u32 srcPass)
	{
		if (dstPass == srcPass)
		{
			return true;
		}

		auto& pass = m_passes[dstPass];
		for (auto& dep : pass.GetDependentPasses())
		{
			if (DependsOnPass(dep, srcPass))
			{
				return true;
			}
		}

		return false;
	}

	void RenderGraph::TraversePassDependenices(RenderPass& pass, u32 stackCount)
	{
		// Check all inputs into this pass. 
		if (pass.IsDepthSencilInputValid())
		{
			ResourceDependPass(pass, pass.GetDepthStencilInput().GetWritePasses(), stackCount, false, false);
		}

		for (auto input : pass.GetAttachmentInputs())
		{
			bool selfDependent = pass.IsDepthSencilOuputValid() ? pass.GetDepthStencilOutput().GetIndex() == input : false;
			if (std::find(pass.GetColorOutputs().begin(), pass.GetColorOutputs().end(), input) != pass.GetColorOutputs().end())
			{
				selfDependent = true;
			}
			if (!selfDependent)
			{
				ResourceDependPass(pass, m_resources[input].GetWritePasses(), stackCount, false, false);
			}
		}

		for (auto input : pass.GetColorInputs())
		{
			if (input != -1)
			{
				ResourceDependPass(pass, m_resources[input].GetWritePasses(), stackCount, false, false);
			}
		}
	}

	void RenderGraph::ResourceDependPass(RenderPass& pass, const std::unordered_set<u32>& writtenPasses, u32 stackCount, bool noCheck, bool ignoreSelf)
	{
		if (!noCheck && writtenPasses.empty())
		{
			ASSERT(false && "[] No pass exists which writes to resource.");
		}

		if (stackCount > m_passes.size())
		{
			ASSERT(false && "[] Cycle detected.");
		}
		for (auto& wPass : writtenPasses)
		{
			if (wPass != pass.GetPassIndex())
			{
				pass.AddDependentPass(wPass);
			}
		}

		++stackCount;

		for (auto& pushedPass : writtenPasses)
		{
			if (ignoreSelf && pushedPass == pass.GetPassIndex())
			{
				continue;
			}
			else if (pushedPass == pass.GetPassIndex())
			{
				ASSERT(false && "[] Pass depends on it self.");
			}

			m_passStack.push_back(pushedPass);
			auto& pass = m_passes[pushedPass];
			TraversePassDependenices(pass, stackCount);
		}
	}

	void RenderGraph::BuildPhysical()
	{
		for (auto& resource : m_resources)
		{
			if (resource.GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				u32 physicalIndex = m_physicalImages.size();

				GPUImage* image = GPUImage::New();
				GPUImageDesc desc = GPUImageDesc::Image2D(resource.TextureInfo.m_info.Width, resource.TextureInfo.m_info.Height, resource.TextureInfo.m_info.Depth, resource.TextureInfo.m_info.Levels,
														  resource.TextureInfo.m_info.Samples, resource.TextureInfo.m_info.Layers, ImageDomain::Physical, ImageLayout::Undefined, 
														  resource.TextureInfo.m_usageFlags, 0, 0, resource.TextureInfo.m_info.Format, ImageType::Image_2D, ImageUsageType::Render_Target, nullptr);
				image->Init(desc);
				image->SetName(resource.GetFullName());
				m_physicalImages.push_back(image);

				GPUImageView* imageView = GPUImageView::New();
				imageView->Init(image);
				m_physicalImageViews.push_back(imageView);

				resource.SetPhysicalIndex(physicalIndex);
			}
		}
	}
}