#include "ispch.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include "Engine/Config/Config.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Graphics/PixelFormatExtensions.h"
#include "Engine/Graphics/Image/GPUImage.h"

RenderGraph* RenderGraph::New()
{
	switch (Module::GraphicsModule::Instance()->GetAPI())
	{
		case GraphicsRendererAPI::Vulkan: return ::New<RenderGraphVulkan>();
	}
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
	for (auto* pass : m_passes)
	{
		::Delete(pass);
	}

	for (auto* resource : m_resources)
	{
		::Delete(resource);
	}
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
		U32 newIndex = (U32)m_passes.size();
		m_passes.emplace_back(::New<RenderPass>(this, newIndex, name, queue));
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
		U32 newIndex = (U32)m_resources.size();
		m_resources.emplace_back(::New<RenderTextureResouce>(name, newIndex));
		m_resourceToIndex[name] = newIndex;
		return static_cast<RenderTextureResouce&>(*m_resources.back());
	}
}

void RenderGraph::SetbackBufferSource(const std::string& name)
{
	m_backBufferSource = name;
}

void RenderGraph::Build()
{
	Validate();

	auto itr = m_resourceToIndex.find(m_backBufferSource);
	if (itr == m_resourceToIndex.end())
		throw std::logic_error("Backbuffer source does not exist.");

	m_passStack.clear();
	m_passDependencies.clear();
	m_passMergeDependencies.clear();
	m_passDependencies.resize(m_passes.size());
	m_passMergeDependencies.resize(m_passes.size());

	// Go from the back buffer to the front of the graph.
	auto& backbufferResource = m_resources[itr->second];

	if (backbufferResource->GetWritePasses().empty())
		throw std::logic_error("No pass exists which writes to resource.");

	for (auto& pass : backbufferResource->GetWritePasses())
	{
		m_passStack.push_back(pass);
	}

	auto tmpPassStack = m_passStack;
	for (auto& pushedPass : tmpPassStack)
	{
		auto& pass = *m_passes[pushedPass];
		TraverseDependencies(pass, 0);
	}

	std::reverse(m_passStack.begin(), m_passStack.end());
	FilterPasses(m_passStack);

	// Now reorder passes to check if a better pipeline can be made from them.
	ReorderPasses(m_passStack);

	// Now, we have a linear list of passes to submit in-order which would obey the dependencies.

	// Figure out which physical resources we need. Here we will alias resources which can trivially alias via renaming.
	// E.g. depth input -> depth output is just one physical attachment, similar with color.
	BuildPhysicalResources();

	BuildPhysicalPasses();

	BuildTransients();

	BuildRenderPassInfo();

	BuildBarriers();

	// Check if the swapchain needs to be blitted to in case the geometry does not match the backbuffer,
	// or the usage of the image makes that impossible.
	m_swapchainPhysicalIndex = m_resources[m_resourceToIndex[m_backBufferSource]]->GetPhysicalIndex();

	auto& backBufferDims = m_physicalResources.Dimensions[m_swapchainPhysicalIndex];

	// If resource is touched in async-compute, we cannot alias with swapchain.
	// If resource is not transient, it's being used in multiple physical passes,
	// we can't use the implicit subpass dependencies for dealing with swapchain.
	bool canAliasBackBuffer = (backBufferDims.Queues & ComputeQueues) == 0 &&
		backBufferDims.Transient;

	// Resources which do not alias with the backbuffer should not be pre-rotated.
	//for (auto& dim : m_physicalResources.Dimensions)
	//	if (&dim != &backBufferDims)
	//		dim.Transform = Idenitiy;

	//LOGI("Backbuffer transform: %u\n", backbuffer_dim.transform);
	//if (Vulkan::surface_transform_swaps_xy(backbuffer_dim.transform))
	//	std::swap(backbuffer_dim.width, backbuffer_dim.height);

	backBufferDims.Transient = false;
	backBufferDims.Persistent = m_swapchainDimensions.Persistent;
	if (!canAliasBackBuffer || backBufferDims != m_swapchainDimensions)
	{
		IS_CORE_WARN("Cannot alias with backbuffer, requires extra blit pass!\n");
		IS_CORE_WARN("  Backbuffer: {0} x {1}, fmt: {2}\n",
			backBufferDims.Width, backBufferDims.Height,
			backBufferDims.Format);
		IS_CORE_WARN("  Swapchain: {0} x {1}, fmt: {2}\n",
			backBufferDims.Width, backBufferDims.Height,
			backBufferDims.Format);

		m_swapchainPhysicalIndex = RenderGraphResource::Unused;
		if ((backBufferDims.Queues & RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT) == 0)
			backBufferDims.Queues |= RENDER_GRAPH_QUEUE_ASYNC_GRAPHICS_BIT;
		else
			backBufferDims.Queues |= RENDER_GRAPH_QUEUE_GRAPHICS_BIT;

		// We will need to sample from the image to blit to backbuffer.
		backBufferDims.ImageUsage |= (U32)ImageUsageFlagsBits::Sampled;

		// Don't use pre-transform if we can't alias anyways.
		//if (Vulkan::surface_transform_swaps_xy(backbuffer_dim.transform))
		//	std::swap(backbuffer_dim.width, backbuffer_dim.height);
		//backbuffer_dim.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
		m_physicalResources.Dimensions[m_swapchainPhysicalIndex].Transient = true;

	// Based on our render graph, figure out the barriers we actually need.
	// Some barriers are implicit (transients), and some are redundant, i.e. same texture read in multiple passes.
	BuildPhysicalBarriers();

	// Figure out which images can alias with each other.
	// Also build virtual "transfer" barriers. These things only copy events over to other physical resources.
	BuidAliases();
}

void RenderGraph::SetupAttachments()
{
	m_physicalResources.Attachments.clear();
	m_physicalResources.Attachments.resize(m_physicalResources.Dimensions.size());

	// Try to reuse the buffers if possible.
	m_physicalResources.Buffers.resize(m_physicalResources.Dimensions.size());

	// Try to reuse render targets if possible.
	m_physicalResources.ImageAttachments.resize(m_physicalResources.Dimensions.size());
	m_physicalResources.HistoryImageAttachments.resize(m_physicalResources.Dimensions.size());
	m_physicalResources.Events.resize(m_physicalResources.Dimensions.size());
	m_physicalResources.HistoryEvents.resize(m_physicalResources.Dimensions.size());

	auto& device = *GPUDevice::Instance();

	U32 numAttachments = (U32)m_physicalResources.Dimensions.size();
	for (U32 i = 0; i < numAttachments; ++i)
	{
		// Move over history attachments and events.
		if (m_physicalResources.PhysicalImageHasHistory[i])
		{
			std::swap(m_physicalResources.HistoryImageAttachments[i], m_physicalResources.ImageAttachments[i]);
			std::swap(m_physicalResources.HistoryEvents[i], m_physicalResources.Events[i]);
		}

		auto& att = m_physicalResources.Dimensions[i];
		if (false/*att.BufferInfo.Size != 0*/)
		{
			SetupPhysicalBuffer(device, i);
		}
		else
		{
			if (att.IsStorageImage())
				SetupPhysicalImage(device, i);
			//else if (i == m_swapchainPhysicalIndex)
				//m_physicalResources.Attachments[i] = device.GetSwapchainView();
			else if (att.Transient)
				m_physicalResources.Attachments[i] = device.GetTransientAttachment(att.Width, att.Height, att.Format, i, att.Samples, att.Layers);
			else
				SetupPhysicalImage(device, i);
		}
	}

	// Assign concrete ImageViews to the render pass.
	for (auto& physicalPass : m_physicalPasses)
	{
		U32 layers = ~0u;

		U32 numColorAttachments = (U32)physicalPass.PhysicalColorAttachments.size();
		for (U32 i = 0; i < numColorAttachments; ++i)
		{
			auto& att = physicalPass.RenderPassInfo.ColorAttachments[i];
			att = m_physicalResources.Attachments[physicalPass.PhysicalColorAttachments[i]];
			if (att->GetImage()->GetDesc().Domain == ImageDomain::Physical)
				layers = std::min(layers, att->GetImage()->GetDesc().Layers);
		}

		if (physicalPass.PhysicalDepthStencilAttachment != RenderGraphResource::Unused)
		{
			auto& ds = physicalPass.RenderPassInfo.DepthStenci;
			ds = m_physicalResources.Attachments[physicalPass.PhysicalDepthStencilAttachment];
			if (ds->GetImage()->GetDesc().Domain == ImageDomain::Physical)
				layers = std::min(layers, ds->GetImage()->GetDesc().Layers);
		}
		else
			physicalPass.RenderPassInfo.DepthStenci = nullptr;

		physicalPass.Layers = layers;
	}
}

void RenderGraph::Reset()
{
	m_passes.clear();
	m_resources.clear();
	m_passToIndex.clear();
	m_resourceToIndex.clear();
	m_physicalPasses.clear();
	m_physicalResources.Dimensions.clear();
	m_physicalResources.Attachments.clear();
	//m_physicalResources.PhysicalBuffers.clear();
	m_physicalResources.ImageAttachments.clear();
	m_physicalResources.Events.clear();
	m_physicalResources.HistoryEvents.clear();
	m_physicalResources.HistoryImageAttachments.clear();
}

void RenderGraph::LogToConsole()
{
	for (auto& resource : m_physicalResources.Dimensions)
	{
		if (false/*resource.GetBufferInfo().Size*/)
		{
			/*IS_INFO("Resource {0} {1}: Size: {2}",
				U32(&resource - m_physicalResources.Dimensions.data()),
				resource.Name, 
				U32(resource.GetBufferInfo().Size)*/
		}
		else
		{
			IS_INFO("Resource {0} {1}: {2} x {3} (fmt: {4}), samples: {5}, transient{6}{7}",
				U32(&resource - m_physicalResources.Dimensions.data()),
				resource.Name,
				resource.Width, resource.Height, U32(resource.Format), resource.Samples, resource.Transient ? "yes" : "no",
				U32(&resource - m_physicalResources.Dimensions.data()) == m_swapchainPhysicalIndex ? " (swapchain)" : "");
		}
	}

	auto barrierItr = m_passBarriers.begin();
	const auto swapStr = [this](const Barrier& barreir) -> const char*
	{
		return barreir.ResourceIndex == m_swapchainPhysicalIndex ? " (swapchain)" : "";
	};

	for (auto& subpasses : m_physicalPasses)
	{
		IS_INFO("Physical pass{0}:", U32(&subpasses - m_physicalPasses.data()));

		for (auto& barrier  : subpasses.Invalidate)
		{
			IS_INFO("	Invalidate: {0}{1}, Layout: {2}, Access: {3}, Stages: {4}",
				barrier.ResourceIndex,
				swapStr(barrier),
				ImageLayoutToString(barrier.Layout),
				AccessFlagsToString(barrier.Access),
				PipelineStageFlagsToString(barrier.Stages));
		}

		for (auto& subpass : subpasses.Passes)
		{
			IS_INFO("		Subpass {0}, ({1}):", 
				U32(&subpass - subpasses.Passes.data()),
				m_passes[subpass]->GetPassName());
			auto& pass = *m_passes[subpass];

			auto& barriers = *barrierItr;
			for (auto& barrier : barriers.Invalidate)
			{
				if (!m_physicalResources.Dimensions[barrier.ResourceIndex].Transient)
				{
					IS_INFO("			Invalidate: {0}{1}, Layout: {2}, Access: {3}, Stages: {4}",
						barrier.ResourceIndex,
						swapStr(barrier),
						ImageLayoutToString(barrier.Layout),
						AccessFlagsToString(barrier.Access),
						PipelineStageFlagsToString(barrier.Stages));
				}
			}

			if (pass.GetDepthStencilOutput())
				IS_INFO("			DepthStencil RW: {0}", pass.GetDepthStencilOutput()->GetPhysicalIndex());
			else if (pass.GetDepthStencilInput())
				IS_INFO("			DepthStencil ReadOnly: {0}", pass.GetDepthStencilInput()->GetPhysicalIndex());

			for (auto& output : pass.GetColorOutputs())
				IS_INFO("			ColorAttachment {0}: {1}", U32(&output - pass.GetColorOutputs().data()), output->GetPhysicalIndex());
		/*	for (auto& output : pass.GetResolveOutputs())
				IS_INFO("			ResolveAttachment {0}: {1}", U32(&output - pass.GetResolveOutputs().data()), output->GetPhysicalIndex());*/
			for (auto& input : pass.GetAttachmentInputs())
				IS_INFO("			InputAttachment {0}: {1}", U32(&input - pass.GetAttachmentInputs().data()), input->GetPhysicalIndex());
			for (auto& input : pass.GetGenericTextureInputs())
				IS_INFO("			Read-onlyTexture {0}: {1}", U32(&input - pass.GetGenericTextureInputs().data()), input.Texture->GetPhysicalIndex());
			//for (auto& input : pass.GetGenericBufferInputs())
			//	IS_INFO("			ColorAttachment {0}: {1}", U32(&input - pass.GetGenericBufferInputs().data()), input.Buffer->GetPhysicalIndex());

			//for (auto& input : pass.GetColorScaleInputs())
			//{
			//	if (input)
			//	{
			//		IS_INFO("			ColorScaleInput {0}: {1}",
			//			I32(&input - pass.GetColorScaleInputs().data()),
			//			input->GetPhysicalIndex());
			//	}
			//}

			for (auto& barrier : barriers.Flush)
			{
				if (!m_physicalResources.Dimensions[barrier.ResourceIndex].Transient &&
					barrier.ResourceIndex != m_swapchainPhysicalIndex)
				{
					IS_INFO("			Flush: {0}{1}, Layout: {2}, Access: {3}, Stages: {4}",
						barrier.ResourceIndex,
						swapStr(barrier),
						ImageLayoutToString(barrier.Layout),
						AccessFlagsToString(barrier.Access),
						PipelineStageFlagsToString(barrier.Stages));
				}
			}
			++barrierItr;
		}

		for (auto& barrier : subpasses.Flush)
		{
			IS_INFO("	Flush: {0}{1}, Layout: {2}, Access: {3}, Stages: {4}",
				barrier.ResourceIndex,
				swapStr(barrier),
				ImageLayoutToString(barrier.Layout),
				AccessFlagsToString(barrier.Access),
				PipelineStageFlagsToString(barrier.Stages));
		}
	}
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

	dim.Width = info.Width == 0 ? m_swapchainDimensions.Width : info.Width;
	dim.Height = info.Height == 0 ? m_swapchainDimensions.Height : info.Height;
	dim.Depth = info.Depth == 0 ? m_swapchainDimensions.Depth : info.Depth;

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

		U32 numInputs = (U32)pass->GetColorInputs().size();
		for (U32 i = 0; i < numInputs; ++i)
		{
			if (!pass->GetColorInputs()[i])
			{
				continue;
			}

			if (GetResourceDimensions(*pass->GetColorInputs()[i]) !=
				GetResourceDimensions(*pass->GetColorOutputs()[i]))
			{
				throw std::logic_error("Dimensions of the color inputs and outputs are a mismatch.");
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

void RenderGraph::TraverseDependencies(const RenderPass& pass, U32 stackCount)
{
	// For these kinds of resources,
	// make sure that we pull in the dependency right away so we can merge render passes if possible.
	if (pass.GetDepthStencilInput())
	{
		DependPassesRecursive(pass, pass.GetDepthStencilInput()->GetWritePasses(),
			stackCount, false, false, true);
	}

	for (auto* input : pass.GetAttachmentInputs())
	{
		bool self_Dependency = pass.GetDepthStencilOutput() == input;
		if (std::find(pass.GetColorOutputs().begin(), pass.GetColorOutputs().end(), input) != pass.GetColorOutputs().end())
			self_Dependency = true;
		if (!self_Dependency)
			DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, true);
	}

	for (auto* input : pass.GetColorInputs())
	{
		if (input)
			DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, true);
	}

	for (auto& input : pass.GetGenericTextureInputs())
	{
		DependPassesRecursive(pass, input.Texture->GetWritePasses(), stackCount, false, false, false);
	}
}

void RenderGraph::DependPassesRecursive(const RenderPass& self, const std::unordered_set<U32>& writtenPasses,
	U32 stackCount, bool noCheck, bool ignoreSelf, bool mergeDependency)
{
	if (!noCheck && writtenPasses.empty())
		throw std::logic_error("No pass exists which writes to resource.");

	if (stackCount > m_passes.size())
		throw std::logic_error("Cycle detected");

	// Check if a resources requires another pass.
	for (auto& pass : writtenPasses)
	{
		if (pass != self.GetIndex())
			m_passDependencies[self.GetIndex()].insert(pass);
	}

	if (mergeDependency)
	{
		for (auto& pass : writtenPasses)
		{
			if (pass != self.GetIndex())
				m_passMergeDependencies[self.GetIndex()].insert(pass);
		}
	}

	++stackCount;

	for (auto& pushedPass : writtenPasses)
	{
		if (ignoreSelf && pushedPass == self.GetIndex())
			continue;
		else if (pushedPass == self.GetIndex())
			throw std::logic_error("Pass depends on itself.");

		m_passStack.push_back(pushedPass);
		auto& pass = *m_passes[pushedPass];
		TraverseDependencies(pass, stackCount);
	}
}

void RenderGraph::FilterPasses(std::vector<U32>& list)
{
	std::unordered_set<U32> seen;
	auto outputItr = list.begin();
	for (auto itr = list.begin(); itr != list.end(); ++itr)
	{
		if (!seen.count(*itr))
		{
			*outputItr = *itr;
			seen.insert(*itr);
			++outputItr;
		}
	}
	list.erase(outputItr, list.end());
}

void RenderGraph::ReorderPasses(std::vector<U32>& passes)
{
	// If a pass depends on an earlier pass via merge dependencies,
	// copy over dependencies to the dependees to avoid cases which can break subpass merging.
	// This is a "soft" dependency. If we ignore it, it's not a real problem.
	for (auto& passMergeDeps : m_passMergeDependencies)
	{
		auto passIndex = U32(&passMergeDeps - m_passMergeDependencies.data());
		auto& passDeps = m_passMergeDependencies[passIndex];

		for (auto& mergeDps : passMergeDeps)
		{
			for (auto& dependee : passDeps)
			{
				if (DependsOnPass(dependee, mergeDps))
					continue;

				if (mergeDps != dependee)
					m_passDependencies[mergeDps].insert(dependee);
			}
		}
	}

	// TODO: This is very inefficient, but should work okay for a reasonable amount of passes ...
	// But, reasonable amounts are always one more than what you'd think ...
	// Clarity in the algorithm is pretty important, because these things tend to be very annoying to debug.
	if (passes.size() <= 2)
	{
		return;
	}

	std::vector<U32> unscheduledPasses;
	unscheduledPasses.reserve(m_passes.size());
	std::swap(passes, unscheduledPasses);

	const auto schedule = [&](U32 index)
	{
		passes.push_back(unscheduledPasses[index]);
		std::move(unscheduledPasses.begin() + index + 1,
			unscheduledPasses.end(),
			unscheduledPasses.begin() + index);
		unscheduledPasses.pop_back();
	};

	schedule(0);
	while (!unscheduledPasses.empty())
	{
		// Find the next pass to schedule.
		// We can pick any pass N, if the pass does not depend on anything left in unscheduled_passes.
		// unscheduled_passes[0] is always okay as a fallback, so unless we find something better,
		// we will at least pick that.

		// Ideally, we pick a pass which does not introduce any hard barrier.
		// A "hard barrier" here is where a pass depends directly on the pass before it forcing something ala vkCmdPipelineBarrier,
		// we would like to avoid this if possible.

		// Find the pass which has the optimal overlap factor which means the number of passes can be scheduled in-between
		// the depender, and the dependee.

		U32 bestCandidate = 0;
		U32 bestOverlapFactor = 0;

		for (U32 i = 0; i < unscheduledPasses.size(); ++i)
		{
			U32 overlapFactor = 0;

			// Always try to merge passes if possible on tilers.
			// This might not make sense on desktop however,
			// so we can conditionally enable this path depending on our GPU.
			if (m_passMergeDependencies[unscheduledPasses[i]].count(passes.back()))
			{
				overlapFactor = ~0u;
			}
			else
			{
				for (auto itr = passes.rbegin(); itr != passes.rend(); ++itr)
				{
					if (DependsOnPass(unscheduledPasses[i], *itr))
						break;
					++overlapFactor;
				}
			}

			if (overlapFactor <= bestOverlapFactor)
			{
				continue;
			}

			bool possibleCandiate = true;
			for (U32 j = 0; j < i; ++j)
			{
				if (DependsOnPass(unscheduledPasses[i], unscheduledPasses[j]))
				{
					possibleCandiate = false;
					break;
				}
			}

			if (!possibleCandiate)
			{
				continue;
			}
			bestCandidate = i;
			bestOverlapFactor = overlapFactor;
		}
		schedule(bestCandidate);
	}
}

bool RenderGraph::DependsOnPass(U32 dstPass, U32 srcPass)
{
	if (dstPass == srcPass)
	{
		return true;
	}

	for (auto& dep : m_passDependencies[dstPass])
	{
		if (DependsOnPass(dep, srcPass))
			return true;
	}

	return false;
}

void RenderGraph::BuildPhysicalResources()
{
	U32 physIndex = 0;

	// Find resources which can alias safely.
	for (auto& passIndex : m_passStack)
	{
		auto& pass = *m_passes[passIndex];

		for (auto& input : pass.GetGenericTextureInputs())
		{
			if (input.Texture->GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				m_physicalResources.Dimensions.push_back(GetResourceDimensions(*input.Texture));
				input.Texture->SetPhysicalIndex(physIndex++);
			}
			else
			{
				m_physicalResources.Dimensions[input.Texture->GetPhysicalIndex()].Queues |= input.Texture->GetUsedQueues();
				m_physicalResources.Dimensions[input.Texture->GetPhysicalIndex()].ImageUsage |= input.Texture->GetImageUsage();
			}
		}

		if (!pass.GetColorInputs().empty())
		{
			U32 size = (U32)pass.GetColorInputs().size();
			for (U32 i = 0; i < size; ++i)
			{
				auto* input = pass.GetColorInputs()[i];
				if (!input)
					continue;

				if (input->GetPhysicalIndex() == RenderGraphResource::Unused)
				{
					m_physicalResources.Dimensions.push_back(GetResourceDimensions(*input));
					input->SetPhysicalIndex(physIndex++);
				}
				else
				{
					m_physicalResources.Dimensions[input->GetPhysicalIndex()].Queues |= input->GetUsedQueues();
					m_physicalResources.Dimensions[input->GetPhysicalIndex()].ImageUsage |= input->GetImageUsage();
				}

				if (pass.GetColorOutputs()[i]->GetPhysicalIndex() == RenderGraphResource::Unused)
					pass.GetColorOutputs()[i]->SetPhysicalIndex(input->GetPhysicalIndex());
				else if (pass.GetColorOutputs()[i]->GetPhysicalIndex() == input->GetPhysicalIndex())
					throw std::logic_error("Cannot alias resource. Index already claimed.");
			}
		}


		for (auto* output : pass.GetColorOutputs())
		{
			if (output->GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				m_physicalResources.Dimensions.push_back(GetResourceDimensions(*output));
				output->SetPhysicalIndex(physIndex++);
			}
			else
			{
				m_physicalResources.Dimensions[output->GetPhysicalIndex()].Queues |= output->GetUsedQueues();
				m_physicalResources.Dimensions[output->GetPhysicalIndex()].ImageUsage |= output->GetImageUsage();
			}
		}


		auto* dsOutput = pass.GetDepthStencilOutput();
		auto* dsInput = pass.GetDepthStencilInput();
		if (dsInput)
		{
			if (dsInput->GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				m_physicalResources.Dimensions.push_back(GetResourceDimensions(*dsInput));
				dsInput->SetPhysicalIndex(physIndex++);
			}
			else
			{
				m_physicalResources.Dimensions[dsInput->GetPhysicalIndex()].Queues |= dsInput->GetUsedQueues();
				m_physicalResources.Dimensions[dsInput->GetPhysicalIndex()].ImageUsage |= dsInput->GetImageUsage();
			}

			if (dsOutput)
			{
				if (dsOutput->GetPhysicalIndex() == RenderGraphResource::Unused)
					dsOutput->SetPhysicalIndex(dsInput->GetPhysicalIndex());
				else if (dsOutput->GetPhysicalIndex() != dsInput->GetPhysicalIndex())
					throw std::logic_error("Cannot alias resources. Index already claimed.");

				m_physicalResources.Dimensions[dsOutput->GetPhysicalIndex()].Queues |= dsOutput->GetUsedQueues();
				m_physicalResources.Dimensions[dsOutput->GetPhysicalIndex()].ImageUsage |= dsOutput->GetImageUsage();
			}
		}
		else if (dsOutput)
		{
			if (dsOutput->GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				m_physicalResources.Dimensions.push_back(GetResourceDimensions(*dsOutput));
				dsOutput->SetPhysicalIndex(physIndex++);
			}
			else
			{
				m_physicalResources.Dimensions[dsOutput->GetPhysicalIndex()].Queues |= dsOutput->GetUsedQueues();
				m_physicalResources.Dimensions[dsOutput->GetPhysicalIndex()].ImageUsage |= dsOutput->GetImageUsage();
			}
		}

		// Assign input attachments last so they can alias properly with existing color/depth attachments in the
		// same subpass.
		for (auto* input : pass.GetAttachmentInputs())
		{
			if (input->GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				m_physicalResources.Dimensions.push_back(GetResourceDimensions(*input));
				input->SetPhysicalIndex(physIndex++);
			}
			else
			{
				m_physicalResources.Dimensions[input->GetPhysicalIndex()].Queues |= input->GetUsedQueues();
				m_physicalResources.Dimensions[input->GetPhysicalIndex()].ImageUsage |= input->GetImageUsage();
			}
		}

		// Figure out which physical resources need to have history.
		m_physicalResources.PhysicalImageHasHistory.clear();
		m_physicalResources.PhysicalImageHasHistory.resize(m_physicalResources.Dimensions.size());

		for (auto& passIndex : m_passStack)
		{
			auto& pass = *m_passes[passIndex];
			for (auto& history : pass.GetHistoryInputs())
			{
				U32 historyPhysIndex = history->GetPhysicalIndex();
				if (historyPhysIndex == RenderGraphResource::Unused)
					throw std::logic_error("History input is used, but it was never written to.");
				m_physicalResources.PhysicalImageHasHistory[historyPhysIndex] = true;
			}
		}
	}
}

void RenderGraph::BuildTransients()
{
	std::vector<U32> physicalPassUsed(m_physicalResources.Dimensions.size());
	for (auto& u : physicalPassUsed)
	{
		u = RenderGraphResource::Unused;
	}

	for (auto& dim : m_physicalResources.Dimensions)
	{
		// Buffers are never transient.
		// Storage images are never transient.
		if (dim.IsBufferLike())
			dim.Transient = false;
		else
			dim.Transient = true;

		U32 index = U32(&dim - m_physicalResources.Dimensions.data());
		if (m_physicalResources.PhysicalImageHasHistory[index])
			dim.Transient = false;

		if (PixelFormatExtensions::IsDepthStencil(dim.Format) && !(bool)CONFIG_VAL(Config::GraphicsConfig.UseTransientDepthStencil))
			dim.Transient = false;
		if (!PixelFormatExtensions::IsDepthStencil(dim.Format) && !(bool)CONFIG_VAL(Config::GraphicsConfig.UseTransientColor))
			dim.Transient = false;
	}

	for (auto& resource : m_resources)
	{
		if (resource->GetType() != RenderGraphResource::Type::Texture)
			continue;

		U32 physicalIndex = resource->GetPhysicalIndex();
		if (physicalIndex == RenderGraphResource::Unused)
			continue;

		for (auto& pass : resource->GetWritePasses())
		{
			U32 phys = m_passes[pass]->GetPhysicalPassIndex();
			if (phys != RenderGraphResource::Unused)
			{
				if (physicalPassUsed[physicalIndex] != RenderGraphResource::Unused &&
					phys != physicalPassUsed[physicalIndex])
				{
					m_physicalResources.Dimensions[physicalIndex].Transient = false;
					break;
				}
				physicalPassUsed[physicalIndex] = phys;
			}
		}

		for (auto& pass : resource->GetReadPasses())
		{
			U32 phys = m_passes[pass]->GetPhysicalPassIndex();
			if (phys != RenderGraphResource::Unused)
			{
				if (physicalPassUsed[physicalIndex] != RenderGraphResource::Unused &&
					phys != physicalPassUsed[physicalIndex])
				{
					m_physicalResources.Dimensions[physicalIndex].Transient = false;
					break;
				}
				physicalPassUsed[physicalIndex] = phys;
			}
		}
	}
}

void RenderGraph::BuildRenderPassInfo()
{
	for (auto& physicalPass : m_physicalPasses)
	{
		auto& rp = physicalPass.RenderPassInfo;
		physicalPass.Subpasses.resize(physicalPass.Passes.size());
		rp.subpass = physicalPass.Subpasses.data();
		rp.ClearAttachments = 0;
		rp.LoadAttachments = 0;
		rp.StoreAttachments = ~0u;
		physicalPass.ColorClearRequests.clear();
		physicalPass.DepthClearRequest = {};

		auto& colors = physicalPass.PhysicalColorAttachments;
		colors.clear();

		const auto addUniqueColor = [&](U32 index) ->std::pair<U32, bool>
		{
			auto itr = std::find(colors.begin(), colors.end(), index);
			if (itr != colors.end())
				return std::make_pair(U32(itr - colors.begin()), false);
			else
			{
				U32 ret = (U32)colors.size();
				colors.push_back(index);
				return std::make_pair(ret, true);
			}
		};

		const auto addUniqueAttachment = [&](U32 index) ->std::pair<U32, bool>
		{
			if (index == physicalPass.PhysicalDepthStencilAttachment)
				return std::make_pair(U32(colors.size()), false);
			else
				return addUniqueColor(index);
		};

		for (auto& subpass : physicalPass.Passes)
		{
			std::vector<ScaledClearRequest> scaledClearRequests;

			auto& pass = *m_passes[subpass];
			auto subpassIndex = U32(&subpass - physicalPass.Passes.data());

			// Add color attachments.
			U32 numColorAttachments = (U32)pass.GetColorOutputs().size();
			physicalPass.Subpasses[subpassIndex].NumColorAttachments = numColorAttachments;
			for (U32 i = 0; i < numColorAttachments; ++i)
			{
				auto res = addUniqueColor(pass.GetColorOutputs()[i]->GetPhysicalIndex());
				physicalPass.Subpasses[subpassIndex].ColorAttachments[i] = res.first;
				// This is the first time the color attachment is used, check if we need LOAD, or if we can clear it.
				if (res.second)
				{
					bool hasColorInput = !pass.GetColorInputs().empty() && pass.GetColorInputs()[i];
					//bool hasScaledColorInput = !pass.GetColorScaledInputs().empty() && pass.GetColorScaledInputs()[i];
					if (!hasColorInput /*&& !hasScaledColorInput*/)
					{
						if (pass.GetClearColor(i))
						{
							rp.ClearAttachments |= 1u << res.first;
							physicalPass.ColorClearRequests.push_back({ &pass, &rp.ClearColor[res.first], (U32)i });
						}
					}
					else
					{
						if (false /*hasScaledColorInput*/) {}
						//scaledClearRequests.push_back({ i, pass.GetColorScaledInputs()[i]->GetPhysicalIndex() });
						else
							rp.LoadAttachments |= 1u << res.first;
					}
				}
			}

			//if (!pass.GetResolveOutputs().empty())
			//{
			//	physicalPass.Subpasses[subpassIndex].NumResolveAttachments = numColorAttachments;
			//	for (U32 i = 0; i < numColorAttachments; ++i)
			//	{
			//		auto res = addUniqueColor(pass.GetResolveOutputs()[i]->GetPhycsialIndex());
			//		physicalPass.Subpasses[subpassIndex].ResolveAttachments[i] = res.first;
			//		// Resolve attachments are don't care always.
			//	}
			//}

			physicalPass.ScaledClearRequests.push_back(std::move(scaledClearRequests));

			auto* dsInput = pass.GetDepthStencilInput();
			auto* dsOutput = pass.GetDepthStencilOutput();

			const auto addUniqueDS = [&](U32 index) -> std::pair<U32, bool>
			{
				ASSERT(physicalPass.PhysicalDepthStencilAttachment == RenderTextureResouce::Unused ||
					physicalPass.PhysicalDepthStencilAttachment == index);
				bool newAttachment = physicalPass.PhysicalDepthStencilAttachment == RenderTextureResouce::Unused;
				physicalPass.PhysicalDepthStencilAttachment = index;
				return std::make_pair(index, newAttachment);
			};

			if (dsOutput && dsInput)
			{
				auto res = addUniqueDS(dsOutput->GetPhysicalIndex());
				// If this is the first subpass the attachment is used, we need to load it.
				if (res.second)
					rp.LoadAttachments |= 1u << res.first;

				rp.OpFlags |= (U32)RenderPassOp::Store_Depth_Stencil;
				physicalPass.Subpasses[subpassIndex].DepthStencilMode = RenderPassInfo::DepthStencil::Read_Write;
			}
			else if (dsOutput)
			{
				auto res = addUniqueDS(dsOutput->GetPhysicalIndex());
				// If this is the first subpass the attachment is used, we need to either clear or discard.
				if (res.second && pass.GetClearDepthStencil())
				{
					rp.OpFlags |= (U32)RenderPassOp::Clear_Depth_Stencil;
					physicalPass.DepthClearRequest.Pass = &pass;
					physicalPass.DepthClearRequest.Target = &rp.ClearDepthStencil;
				}

				rp.OpFlags |= (U32)RenderPassOp::Store_Depth_Stencil;
				physicalPass.Subpasses[subpassIndex].DepthStencilMode = RenderPassInfo::DepthStencil::Read_Write;

				ASSERT(physicalPass.PhysicalDepthStencilAttachment == RenderTextureResouce::Unused ||
					physicalPass.PhysicalDepthStencilAttachment == dsOutput->GetPhysicalIndex())
					physicalPass.PhysicalDepthStencilAttachment = dsOutput->GetPhysicalIndex();
			}
			else if (dsInput)
			{
				auto res = addUniqueDS(dsInput->GetPhysicalIndex());

				// If this is the first subpass the attachment is used, we need to load.
				if (res.second)
				{
					rp.OpFlags |= (U32)RenderPassOp::Depth_Stencil_Read_Only |
						(U32)RenderPassOp::Load_Depth_Stencil;

					bool preserveDepth = false;
					for (auto& readPass : dsInput->GetReadPasses())
					{
						if (m_passes[readPass]->GetPhysicalPassIndex() > U32(&physicalPass - m_physicalPasses.data()))
						{
							preserveDepth = true;
							break;
						}
					}

					if (preserveDepth)
					{
						// Have to store here, or the attachment becomes undefined in future passes.
						rp.OpFlags |= (U32)RenderPassOp::Store_Depth_Stencil;
					}
				}
				physicalPass.Subpasses[subpassIndex].DepthStencilMode = RenderPassInfo::DepthStencil::Read_Only;
			}
			else
			{
				physicalPass.Subpasses[subpassIndex].DepthStencilMode = RenderPassInfo::DepthStencil::None;
			}
		}

		for (auto& subpass : physicalPass.Passes)
		{
			auto& pass = *m_passes[subpass];
			U32 subpassIndex = U32(&subpass - physicalPass.Passes.data());

			// Add input attachments.
			// Have to do these in a separate loop so we can pick up depth stencil input attachments properly.
			U32 numInputAttachments = (U32)pass.GetAttachmentInputs().size();
			physicalPass.Subpasses[subpassIndex].NumInputAttachments = numInputAttachments;
			for (U32 i = 0; i < numInputAttachments; ++i)
			{
				auto res = addUniqueAttachment(pass.GetAttachmentInputs()[i]->GetPhysicalIndex());
				physicalPass.Subpasses[subpassIndex].InputAttachments[i] = res.first;

				// If this is the first subpass the attachment is used, we need to load it.
				if (res.second)
					rp.LoadAttachments |= 1u << res.first;
			}
		}

		physicalPass.RenderPassInfo.NumColorAttachments = (U32)physicalPass.PhysicalColorAttachments.size();
	}
}

void RenderGraph::BuildBarriers()
{
	m_passBarriers.clear();
	m_passBarriers.reserve(m_passStack.size());

	const auto getAccess = [&](std::vector<Barrier>& barriers, U32 index, bool history) -> Barrier&
	{
		auto itr = std::find_if(barriers.begin(), barriers.end(), [index, history](const Barrier& b)
			{
				return index == b.ResourceIndex && history == b.History;
			});
		if (itr != barriers.end())
			return *itr;
		else
		{
			barriers.push_back({ index, ImageLayout::Undefined, 0, 0, history });
			return barriers.back();
		}
	};

	for (auto& index : m_passStack)
	{
		auto& pass = *m_passes[index];
		Barriers barriers;

		const auto getInvaliateAccess = [&](U32 i, bool history) -> Barrier&
		{
			return getAccess(barriers.Invalidate, i, history);
		};
		const auto getFlushAccess = [&](U32 i) -> Barrier&
		{
			return getAccess(barriers.Flush, i, false);
		};

		/*for (auto& intput : pass.GetGenericBufferInputs())
		{
			auto& barrier = getInvaliateAccess(input.Buffer->GetPhysicalIndex(), false);
			barrier.Access |= input.Access;
			barrier.Stages |= input.Stages;
			if (barrier.Layout != (U32)ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			barrier.Layout = input.Layout;
		}*/

		for (auto& input : pass.GetGenericTextureInputs())
		{
			auto& barrier = getInvaliateAccess(input.Texture->GetPhysicalIndex(), false);
			barrier.Access |= input.Access;
			barrier.Stages |= input.Stages;
			if (barrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			barrier.Layout = (ImageLayout)input.Layout;
		}

		for (auto* input : pass.GetHistoryInputs())
		{
			auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), true);
			barrier.Access |= (U32)Access::Shader_Read;
			if ((pass.GetQueue() & ComputeQueues) == 0)
				barrier.Stages |= (U32)PipelineStage::Fragment_Shader; // TODO: Pick appropriate stage.
			else
				barrier.Stages |= (U32)PipelineStage::Compute_Shader;

			if (barrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			barrier.Layout = ImageLayout::Shader_Read_Only;
		}

		for (auto* input : pass.GetAttachmentInputs())
		{
			if (pass.GetQueue() & ComputeQueues)
				throw std::logic_error("Only graphics passes can have input attachments.");

			auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
			barrier.Access |= (U32)Access::Input_Attachmnet_Read;
			barrier.Stages |= (U32)PipelineStage::Fragment_Shader;
			if (barrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			barrier.Layout = ImageLayout::Shader_Read_Only;
		}

		//for (auto* input : pass.GetStorageInputs())
		//{
		//	if (!input)
		//		continue;

		//	auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
		//	barrier.Access |= (U32)Access::Shader_Read | (U32)Access::Shader_Write;
		//	if ((pass.GetQueue() & ComputeQueues) == 0)
		//		barrier.Stages |= (U32)PipelineStage::Fragment_Shader; // TODO: Pick appropriate stage.
		//	else
		//		barrier.Stages |= (U32)PipelineStage::Compute_Shader;

		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch");
		//	barrier.Layout = (U32)ImageLayout::General;
		//}

		//for (auto* input : pass.GetStorageTextureInputs())
		//{
		//	if (!input)
		//		continue;

		//	auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
		//	barrier.Access |= (U32)Access::Shader_Read | (U32)Access::Shader_Write;
		//	if ((pass.GetQueue() & ComputeQueues) == 0)
		//		barrier.Stages |= (U32)PipelineStage::Fragment_Shader; // TODO: Pick appropriate stage.
		//	else
		//		barrier.Stages |= (U32)PipelineStage::Compute_Shader;

		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch");
		//	barrier.Layout = (U32)ImageLayout::General;
		//}

		//for (auto* input : pass.GetBlitTextureInputs())
		//{
		//	if (!input)
		//		continue;

		//	auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
		//	barrier.Access |= (U32)Access::Transfer_Write;
		//	barrier.Stages |= (U32)PipelineStage::Transfer;
		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch");
		//	barrier.Layout = (U32)ImageLayout::Transfer_Dst;
		//}

		for (auto* input : pass.GetColorInputs())
		{
			if (!input)
				continue;

			if (pass.GetQueue() & ComputeQueues)
				throw std::logic_error("Only graphics passes can have color inputs.");

			auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
			barrier.Access |= (U32)Access::Color_Attachment_Write | (U32)Access::Color_Attachment_Read;
			barrier.Stages |= (U32)PipelineStage::Color_Attachment_Output;

			// If the attachment is also bound as an input attachment (programmable blending)
			// we need VK_IMAGE_LAYOUT_GENERAL.
			if (barrier.Layout == ImageLayout::Shader_Read_Only)
				barrier.Layout = ImageLayout::General;
			else if (barrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			else
				barrier.Layout = ImageLayout::Color_Attachment;
		}

		//for (auto* input : pass.GetColorScaleInputs())
		//{
		//	if (!input)
		//		continue;

		//	if (pass.GetQueue() & ComputeQueues)
		//		throw std::logic_error("Only graphics passes can have color inputs.");

		//	auto& barrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
		//	barrier.Access |= (U32)Access::Shader_Read;
		//	barrier.Stages |= (U32)PipelineStage::Fragment_Shader;
		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch");
		//	barrier.Layout = (U32)ImageLayout::Shader_Read_Only;
		//}

		for (auto* output : pass.GetColorOutputs())
		{
			if (pass.GetQueue() & ComputeQueues)
				throw std::logic_error("Only graphics passes can have color outputs.");

			auto& barrier = getFlushAccess(output->GetPhysicalIndex());
			if (m_physicalResources.Dimensions[output->GetPhysicalIndex()].Levels > 1)
			{
				// access should be 0 here. generate_mipmaps will take care of invalidation needed.
				barrier.Access |= (U32)Access::Transfer_Read;
				barrier.Stages |= (U32)PipelineStage::Transfer;
				if (barrier.Layout != ImageLayout::Undefined)
					throw std::logic_error("Layout mismatch");
				barrier.Layout = ImageLayout::Transfer_Src;
			}
			else
			{
				barrier.Access |= (U32)Access::Color_Attachment_Write;
				barrier.Stages |= (U32)PipelineStage::Color_Attachment_Output;

				// If the attachment is also bound as an input attachment (programmable blending)
				// we need VK_IMAGE_LAYOUT_GENERAL.
				if (barrier.Layout == ImageLayout::Shader_Read_Only ||
					barrier.Layout == ImageLayout::General)
				{
					barrier.Layout = ImageLayout::General;
				}
				else if (barrier.Layout != ImageLayout::Undefined)
					throw std::logic_error("Layout mismatch");
				else
					barrier.Layout = ImageLayout::Color_Attachment;
			}
		}

		//for (auto* output : pass.GetResolveOutputs())
		//{
		//	if (pass.GetQueue() & ComputeQueues)
		//		throw std::logic_error("Only graphics passes can resolve outputs.");

		//	auto& barrier = getFlushAccess(output->GetPhysicalIndex());
		//	barrier.Access |= (U32)Access::Color_Attachment_Write;
		//	barrier.Stages |= (U32)PipelineStage::Color_Attachment_Output;
		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch.");
		//	barrier.layout = (U32)ImageLayout::Color_Attachment;
		//}

		//for (auto* output : pass.GetBlitTextureOutputs())
		//{
		//	auto& barrier = getInvaliateAccess(output->GetPhysicalIndex(), false);
		//	barrier.Access |= (U32)Access::Transfer_Write;
		//	barrier.Stages |= (U32)PipelineStage::Transfer;
		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch");
		//	barrier.Layout = (U32)ImageLayout::Transfer_Dst;
		//}

		//for (auto* output : pass.GetStorageOutputs())
		//{
		//	auto& barrier = getFlushAccess(output->GetPhysicalIndex());
		//	barrier.Access |= (U32)Access::Shader_Write;

		//	if ((pass.GetQueue() & ComputeQueues) == 0)
		//		barrier.Stages |= (U32)PipelineStage::Fragment_Shader; // TODO: Pick appropriate stage.
		//	else
		//		barrier.Stages |= (U32)PipelineStage::Compute_Shader;

		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch.");
		//	barrier.Layout = (U32)ImageLayout::General;
		//}

		//for (auto* output : pass.GetTransferOutputs())
		//{
		//	auto& barrier = getFlushAccess(output->GetPhysicalIndex());
		//	barrier.Access |= (U32)Access::Transfer_Write;
		//	barrier.Stages |= (U32)PipelineStage::Transfer;
		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch.");
		//	barrier.Layout = (U32)ImageLayout::General;
		//}

		//for (auto* output : pass.GetStorageTextureOutputs())
		//{
		//	auto& barrier = getFlushAccess(output->GetPhysicalIndex());
		//	barrier.Access |= (U32)Access::Shader_Write;

		//	if ((pass.GetQueue() & ComputeQueues) == 0)
		//		barrier.Stages |= (U32)PipelineStage::Fragment_Shader; // TODO: Pick appropriate stage.
		//	else
		//		barrier.Stages |= (U32)PipelineStage::Compute_Shader;

		//	if (barrier.Layout != (U32)ImageLayout::Undefined)
		//		throw std::logic_error("Layout mismatch.");
		//	barrier.Layout = (U32)ImageLayout::General;
		//}

		auto* output = pass.GetDepthStencilOutput();
		auto* input = pass.GetDepthStencilInput();
		if ((output || input) && (pass.GetQueue() & ComputeQueues))
			throw std::logic_error("Only graphics passes can have depth attachments.");

		if (output && input)
		{
			auto& dstBarrier = getInvaliateAccess(input->GetPhysicalIndex(), false);
			auto& srcBarrier = getFlushAccess(output->GetPhysicalIndex());

			if (dstBarrier.Layout == ImageLayout::Shader_Read_Only)
				dstBarrier.Layout = ImageLayout::General;
			else if (dstBarrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			else
				dstBarrier.Layout = ImageLayout::Depth_Stencil_Attachment;

			dstBarrier.Access |= (U32)Access::Depth_Stencil_Attachment_Read | (U32)Access::Depth_Stencil_Attachment_Write;
			dstBarrier.Stages |= (U32)PipelineStage::Early_Fragment_Test | (U32)PipelineStage::Late_Fragment_Test;

			srcBarrier.Layout = ImageLayout::Depth_Stencil_Attachment;
			srcBarrier.Access |= (U32)Access::Depth_Stencil_Attachment_Write;
			srcBarrier.Stages |= (U32)PipelineStage::Late_Fragment_Test;
		}
		else if (input)
		{
			auto& dstBarrier = getInvaliateAccess(input->GetPhysicalIndex(), false);

			if (dstBarrier.Layout == ImageLayout::Shader_Read_Only)
				dstBarrier.Layout = ImageLayout::Depth_Stencil_Read_Only;
			else if (dstBarrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			else
				dstBarrier.Layout = ImageLayout::Depth_Stencil_Read_Only;

			dstBarrier.Access |= (U32)Access::Depth_Stencil_Attachment_Read;
			dstBarrier.Stages |= (U32)PipelineStage::Early_Fragment_Test | (U32)PipelineStage::Late_Fragment_Test;
		}
		else if (output)
		{
			auto& srcBarrier = getFlushAccess(output->GetPhysicalIndex());

			if (srcBarrier.Layout == ImageLayout::Shader_Read_Only)
				srcBarrier.Layout = ImageLayout::General;
			else if (srcBarrier.Layout != ImageLayout::Undefined)
				throw std::logic_error("Layout mismatch");
			else
				srcBarrier.Layout = ImageLayout::Depth_Stencil_Attachment;

			srcBarrier.Access |= (U32)Access::Depth_Stencil_Attachment_Write;
			srcBarrier.Stages |= (U32)PipelineStage::Late_Fragment_Test;
		}
		m_passBarriers.push_back(std::move(barriers));
	}
}

void RenderGraph::BuildPhysicalBarriers()
{
	auto barrierItr = m_passBarriers.begin();

	const auto flushAccessToInvalidate = [](AccessFlags flags) -> AccessFlags
	{
		if (flags & (U32)Access::Color_Attachment_Write)
			flags |= (U32)Access::Color_Attachment_Read;
		if (flags & (U32)Access::Depth_Stencil_Attachment_Write)
			flags |= (U32)Access::Depth_Stencil_Attachment_Read;
		if (flags & (U32)Access::Shader_Write)
			flags |= (U32)Access::Shader_Read;
		return flags;
	};

	struct ResourceState
	{
		ImageLayout InitalLayout = ImageLayout::Undefined;
		ImageLayout FinalLayout = ImageLayout::Undefined;
		AccessFlags InvalidatedTypes = 0;
		AccessFlags FlushedTypes = 0;
		PipelineStageFlags InvalidatedStages = 0;
		PipelineStageFlags FlushedStages = 0;
	};

	// To handle state inside a physical pass.
	std::vector<ResourceState> resourceState;
	resourceState.reserve(m_physicalResources.Dimensions.size());

	for (auto& physicalPass : m_physicalPasses)
	{
		resourceState.clear();
		resourceState.resize(m_physicalResources.Dimensions.size());

		// Go over all physical passes, and observe their use of barriers.
		// In multipass, only the first and last barriers need to be considered externally.
		// Compute never has multipass.
		U32 subpasses = (U32)physicalPass.Passes.size();
		for (U32 i = 0; i < subpasses; ++i, ++barrierItr)
		{
			auto& barriers = *barrierItr;
			auto& invalidates = barriers.Invalidate;
			auto& flushes = barriers.Flush;
		
			for (auto& invalidate : invalidates)
			{
				auto& res = resourceState[invalidate.ResourceIndex];

				// Transients and swapchain images are handled implicitly.
				if (m_physicalResources.Dimensions[invalidate.ResourceIndex].Transient ||
					invalidate.ResourceIndex == m_swapchainPhysicalIndex)
				{
					continue;
				}

				if (invalidate.History)
				{
					auto itr = std::find_if(physicalPass.Invalidate.begin(), physicalPass.Invalidate.end(), [&](const Barrier& b) -> bool
						{
							return b.ResourceIndex == invalidate.ResourceIndex && b.History;
						});

					if (itr == physicalPass.Invalidate.end())
					{
						// Storage images should just be in GENERAL all the time instead of SHADER_READ_ONLY_OPTIMAL.
						auto layout = m_physicalResources.Dimensions[invalidate.ResourceIndex].IsStorageImage() ?
							ImageLayout::General : invalidate.Layout;

						// Special case history barriers. They are a bit different from other barriers.
						// We just need to ensure the layout is right and that we avoid write-after-read.
						// Even if we see these barriers in multiple render passes, they will not emit multiple barriers.
						physicalPass.Invalidate.push_back(
							{ invalidate.ResourceIndex, layout, invalidate.Access, invalidate.Stages, true });
						physicalPass.Flush.push_back(
							{invalidate.ResourceIndex, layout, 0, invalidate.Stages, true});
					}
					continue;
				}

				// Only the first use of a resource in a physical pass needs to be handled externally.
				if (res.InitalLayout == ImageLayout::Undefined)
				{
					res.InvalidatedTypes |= invalidate.Access;
					res.InvalidatedStages |= invalidate.Stages;

					// Storage images should just be in GENERAL all the time instead of SHADER_READ_ONLY_OPTIMAL.
					if (m_physicalResources.Dimensions[invalidate.ResourceIndex].IsStorageImage())
						res.InitalLayout = ImageLayout::General;
					else
						res.InitalLayout = (ImageLayout)invalidate.Layout;
				}

				// A read-only invalidation can change the layout.
				if (m_physicalResources.Dimensions[invalidate.ResourceIndex].IsStorageImage())
					res.FinalLayout = ImageLayout::General;
				else 
					res.FinalLayout = (ImageLayout)invalidate.Layout;
				
				// All pending flushes have been invalidated in the appropriate stages already.
				// This is relevant if the invalidate happens in subpass #1 and beyond.
				res.FlushedTypes = 0;
				res.FlushedStages = 0;
			}

			for (auto& flush : flushes)
			{
				auto& res = resourceState[flush.ResourceIndex];

				// Transients are handled implicitly.
				if (m_physicalResources.Dimensions[flush.ResourceIndex].Transient ||
					flush.ResourceIndex == m_swapchainPhysicalIndex)
				{
					continue;
				}

				// The last use of a resource in a physical pass needs to be handled externally.
				res.FlushedTypes |= flush.Access;
				res.FlushedStages |= flush.Stages;

				// Storage images should just be in GENERAL all the time instead of SHADER_READ_ONLY_OPTIMAL.
				if (m_physicalResources.Dimensions[flush.ResourceIndex].IsStorageImage())
					res.FinalLayout = ImageLayout::General;
				else
					res.FinalLayout = (ImageLayout)flush.Layout;

				// If we didn't have an invalidation before first flush, we must invalidate first.
				// Only first flush in a render pass needs a matching invalidation.
				if (res.InitalLayout == ImageLayout::Undefined)
				{
					// If we end in TRANSFER_SRC_OPTIMAL, we actually start in COLOR_ATTACHMENT_OPTIMAL.
					if ((ImageLayout)flush.Layout == ImageLayout::Transfer_Src)
					{
						res.InitalLayout = ImageLayout::Color_Attachment;
						res.InvalidatedStages = (U32)PipelineStage::Color_Attachment_Output;
						res.InvalidatedTypes = (U32)Access::Color_Attachment_Write | (U32)Access::Color_Attachment_Read;
					}
					else
					{
						res.InitalLayout = (ImageLayout)flush.Layout;
						res.InvalidatedStages = flush.Stages;
						res.InvalidatedTypes = flushAccessToInvalidate(flush.Access);
					}

					// We're not reading the resource in this pass, so we might as well transition from UNDEFINED to discard the resource.
					physicalPass.Discards.push_back(flush.ResourceIndex);
				}
			}
		}

		// Now that the render pass has been studied, look at each resource individually and see how we need to deal
		// with the physical render pass as a whole.
		for (auto& resource : resourceState)
		{
			// Resource was not touched in this pass.
			if (resource.FinalLayout == ImageLayout::Undefined && resource.InitalLayout == ImageLayout::Undefined)
				continue;

			ASSERT(resource.FinalLayout != ImageLayout::Undefined);

			U32 index = U32(&resource - resourceState.data());

			physicalPass.Invalidate.push_back(
				{index, resource.InitalLayout, resource.InvalidatedTypes, resource.InvalidatedStages, false});
		
			if (resource.FlushedTypes)
			{
				// Did the pass write anything in this pass which needs to be flushed?
				physicalPass.Flush.push_back({ index, resource.FinalLayout, resource.FlushedTypes, resource.FlushedStages, false });
			}
			else if (resource.InvalidatedTypes)
			{
				// Did the pass read anything in this pass which needs to be protected before it can be written?
				// Implement this as a flush with 0 access bits.
				// This is how Vulkan essentially implements a write-after-read hazard.
				// The only purpose of this flush barrier is to set the last pass which the resource was used as a stage.
				// Do not clear last_invalidate_pass, because we can still keep tacking on new access flags, etc.
				physicalPass.Flush.push_back({ index, resource.FinalLayout, 0, resource.InvalidatedStages, false });
			}

			// If we end in TRANSFER_SRC_OPTIMAL, this is a sentinel for needing mipmapping, so enqueue that up here.
			if (resource.FinalLayout == ImageLayout::Transfer_Src)
			{
				physicalPass.MipmapRequests.push_back({ index, 
					(U32)PipelineStage::Color_Attachment_Output,
					(U32)Access::Color_Attachment_Write, 
					ImageLayout::Color_Attachment });
			}
		}
	}
}

void RenderGraph::BuidAliases()
{
	struct Range
	{
		U32 FirstWritePass = 0;
		U32 LastWritePass = 0;
		U32 FirstReadPass = 0;
		U32 LastReadPass = 0;
		bool blockAlias = false;

		bool HasWritrer() const { return FirstWritePass <= LastWritePass; }
		bool HasReader() const { return FirstReadPass <= LastReadPass; }
		bool IsUsed() const { return HasWritrer() || HasReader(); }

		bool CanAlias() const
		{
			if (HasReader() && HasWritrer() && FirstReadPass <= FirstWritePass)
				return false;
			if (blockAlias)
				return false;
			return true;
		}

		U32 LastUsedPass() const
		{
			U32 lastPass = 0;
			if (HasWritrer())
				lastPass = std::max(lastPass, LastWritePass);
			if (HasReader())
				lastPass = std::max(lastPass, LastReadPass);
			return lastPass;
		}

		U32 FirstUsedPass() const
		{
			U32 firstPass = ~0u;
			if (HasWritrer())
				firstPass = std::min(firstPass, FirstWritePass);
			if (HasReader())
				firstPass = std::min(firstPass, FirstReadPass);
			return firstPass;
		}

		bool DisjointLifetime(const Range& range) const
		{
			if (!IsUsed() || !range.IsUsed())
				return false;
			if (!CanAlias() || !range.CanAlias())
				return false;

			bool left = LastUsedPass() < range.FirstUsedPass();
			bool right = range.LastUsedPass() < FirstUsedPass();
			return left || right;
		}
	};

	std::vector<Range> passRanges(m_physicalResources.Dimensions.size());

	const auto registerReader = [&passRanges](const RenderTextureResouce* resource, U32 passIndex)
	{
		if (resource && passIndex != RenderGraphResource::Unused)
		{
			U32 phys = resource->GetPhysicalIndex();
			if (phys != RenderGraphResource::Unused)
			{
				auto& range = passRanges[phys];
				range.LastReadPass = std::max(range.LastReadPass, passIndex);
				range.FirstReadPass = std::max(range.FirstReadPass, passIndex);
			}
		}
	};

	const auto registerWriter = [&passRanges](const RenderTextureResouce* resource, U32 passIndex, bool blockAlias)
	{
		if (resource && passIndex != RenderGraphResource::Unused)
		{
			U32 phys = resource->GetPhysicalIndex();
			if (phys != RenderGraphResource::Unused)
			{
				auto& range = passRanges[phys];
				range.LastWritePass = std::max(range.LastWritePass, passIndex);
				range.FirstWritePass = std::max(range.FirstWritePass, passIndex);
				if (blockAlias)
					range.blockAlias = blockAlias;
			}
		}
	};

	for (auto& pass : m_passStack)
	{
		auto& subpass = *m_passes[pass];

		for (auto* input : subpass.GetColorInputs())
			registerReader(input, subpass.GetPhysicalPassIndex());
		//for (auto* input : subpass.get_color_scale_inputs())
		//	registerReader(input, subpass.GetPhysicalPassIndex());
		for (auto* input : subpass.GetAttachmentInputs())
			registerReader(input, subpass.GetPhysicalPassIndex());
		for (auto& input : subpass.GetGenericTextureInputs())
			registerReader(input.Texture, subpass.GetPhysicalPassIndex());
		//for (auto* input : subpass.get_blit_texture_inputs())
		//	registerReader(input, subpass.GetPhysicalPassIndex());
		//for (auto* input : subpass.get_storage_texture_inputs())
		//	registerReader(input, subpass.GetPhysicalPassIndex());
		if (subpass.GetDepthStencilInput())
			registerReader(subpass.GetDepthStencilInput(), subpass.GetPhysicalPassIndex());
	
		// If a subpass may not execute, we cannot alias with that resource because some other pass may invalidate it.
		bool block_alias = false;

		if (subpass.GetDepthStencilOutput())
			registerWriter(subpass.GetDepthStencilOutput(), subpass.GetPhysicalPassIndex(), block_alias);
		for (auto* output : subpass.GetColorOutputs())
			registerWriter(output, subpass.GetPhysicalPassIndex(), block_alias);
		//for (auto* output : subpass.get_resolve_outputs())
		//	registerWriter(output, subpass.GetPhysicalPassIndex(), block_alias);
		//for (auto* output : subpass.get_blit_texture_outputs())
		//	registerWriter(output, subpass.GetPhysicalPassIndex(), block_alias);

		// Storage textures are not aliased, because they are implicitly preserved.
		//for (auto* output : subpass.get_storage_texture_outputs())
		//	registerWriter(output, subpass.GetPhysicalPassIndex(), true);
	}

	std::vector<std::vector<U32>> aliasChains(m_physicalResources.Dimensions.size());

	m_physicalResources.Aliases.resize(m_physicalResources.Dimensions.size());
	for (auto& v : m_physicalResources.Aliases)
		v = RenderGraphResource::Unused;

	for (U32 i = 0; i < m_physicalResources.Dimensions.size(); ++i)
	{
		// No aliases for buffers.
		//if (m_physicalResources.Dimensions[i].BufferInfo.Size)
		//	continue;

		// No aliases for images with history.
		if (m_physicalResources.PhysicalImageHasHistory[i])
			continue;

		// Only try to alias with lower-indexed resources, because we allocate them one-by-one starting from index 0.
		for (unsigned j = 0; j < i; ++j)
		{
			if (m_physicalResources.PhysicalImageHasHistory[j])
				continue;

			if (m_physicalResources.Dimensions[i] == m_physicalResources.Dimensions[j])
			{
				// Only alias if the resources are used in the same queue, this way we avoid introducing
				// multi-queue shenanigans. We can only use events to pass aliasing barriers.
				// Also, only alias if we have one single queue.
				bool sameSingleQueue = m_physicalResources.Dimensions[i].Queues == m_physicalResources.Dimensions[j].Queues;
				if ((m_physicalResources.Dimensions[i].Queues & (m_physicalResources.Dimensions[i].Queues - 1)) != 0)
					sameSingleQueue = false;

				if (passRanges[i].DisjointLifetime(passRanges[j]) && sameSingleQueue)
				{
					// We can alias!
					m_physicalResources.Aliases[i] = j;
					if (aliasChains[j].empty())
						aliasChains[j].push_back(j);
					aliasChains[j].push_back(i);

					// We might have different image usage, propagate this information.
					auto merged_image_usage =
						m_physicalResources.Dimensions[j].ImageUsage |= m_physicalResources.Dimensions[i].ImageUsage;
					m_physicalResources.Dimensions[i].ImageUsage = merged_image_usage;
					m_physicalResources.Dimensions[j].ImageUsage = merged_image_usage;
					break;
				}
			}
		}
	}

	// Now we've found the aliases, so set up the transfer barriers in order of use.
	for (auto& chain : aliasChains)
	{
		if (chain.empty())
			continue;

		std::sort(chain.begin(), chain.end(), [&](unsigned a, unsigned b) -> bool {
			return passRanges[a].LastUsedPass() < passRanges[b].FirstUsedPass();
			});

		for (unsigned i = 0; i < chain.size(); i++)
		{
			if (i + 1 < chain.size())
				m_physicalPasses[passRanges[chain[i]].LastUsedPass()].AliasTransfer.push_back(std::make_pair(chain[i], chain[i + 1]));
			else
				m_physicalPasses[passRanges[chain[i]].LastUsedPass()].AliasTransfer.push_back(std::make_pair(chain[i], chain[0]));
		}
	}
}

void RenderGraph::SetupPhysicalBuffer(GPUDevice& device, U32 index)
{
}

void RenderGraph::SetupPhysicalImage(GPUDevice& device, U32 index)
{
	auto& att = m_physicalResources.Dimensions[index];

	if (m_physicalResources.Aliases[index] != RenderGraphResource::Unused)
	{
		m_physicalResources.ImageAttachments[index] = m_physicalResources.ImageAttachments[m_physicalResources.Aliases[index]];
		m_physicalResources.Attachments[index] = m_physicalResources.ImageAttachments[index]->GetView();
		m_physicalResources.Events[index] = {};
		return;
	}

	bool needImage = true;
	ImageUsageFlags usage = att.ImageUsage;
	ImageMiscFlags misc = 0;
	ImageCreateFlags flags = 0;

	if (att.UnormSRGB)
		misc |= (U32)ImageMisc::Mutable_SRGB;
	if (att.IsStorageImage())
		flags |= (U32)ImageCreate::Mutable_Format;

	if (m_physicalResources.ImageAttachments[index])
	{
		if (att.Persistent && 
			m_physicalResources.ImageAttachments[index]->GetDesc().Format == att.Format &&
			m_physicalResources.ImageAttachments[index]->GetDesc().Width == att.Width &&
			m_physicalResources.ImageAttachments[index]->GetDesc().Height == att.Height &&
			m_physicalResources.ImageAttachments[index]->GetDesc().Depth == att.Depth &&
			m_physicalResources.ImageAttachments[index]->GetDesc().Samples == att.Samples &&
			(m_physicalResources.ImageAttachments[index]->GetDesc().Usage & usage) == usage &&
			(m_physicalResources.ImageAttachments[index]->GetDesc().CreateFlags & flags) == flags)
		{
			needImage = false;
		}
	}

	if (needImage)
	{
		GPUImageDescription desc;
		desc.Format = att.Format;
		desc.Type = att.Depth > 1 ? ImageType::Image_3D : ImageType::Image_2D;
		desc.Width = att.Width;
		desc.Height = att.Height;
		desc.Depth = att.Depth;
		desc.Domain = ImageDomain::Physical;
		desc.Levels = att.Levels;
		desc.Layers = att.Layers;
		desc.Usage = usage;
		desc.InitLayout = ImageLayout::Undefined;
		desc.Samples = att.Samples;
		desc.CreateFlags = flags;
	
		if (PixelFormatExtensions::IsDepthStencil(desc.Format))
			desc.Usage &= ~(U32)ImageUsageFlagsBits::Color_Attachment;

		desc.MiscFlags = misc;
		if (att.Queues & (RENDER_GRAPH_QUEUE_GRAPHICS_BIT | RENDER_GRAPH_QUEUE_COMPUTE_BIT))
			desc.MiscFlags |= (U32)ImageMisc::Concurrent_Queue_Graphics;
		if (att.Queues & RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT)
			desc.MiscFlags |= (U32)ImageMisc::Concurrent_Queue_Async_Compute;
		if (att.Queues & RENDER_GRAPH_QUEUE_ASYNC_GRAPHICS_BIT)
			desc.MiscFlags |= (U32)ImageMisc::Concurrent_Queue_Async_Graphics;

		auto* image = GPUImage::New();
		image->Init(desc);
		GPUImageView::New()->Init(image);

		m_physicalResources.ImageAttachments[index] = image;

		// Just keep storage images in GENERAL layout.
		// There is no reason to try enabling compression.
		if (!m_physicalResources.ImageAttachments[index])
			IS_ERROR("[RenderGraph::SetupPhysicalImage] Failed to create image.");
		if (att.IsStorageImage())
			m_physicalResources.ImageAttachments[index]->SetLayout(ImageLayout::General);
		m_physicalResources.ImageAttachments[index]->SetName(att.Name);
		m_physicalResources.Events[index] = {};
	}

	m_physicalResources.Attachments[index] = m_physicalResources.ImageAttachments[index]->GetView();
}

void RenderGraph::BuildPhysicalPasses()
{
	m_physicalPasses.clear();
	PhysicalPass physicalPass;

	// Check if we have found a physical resource.
	const auto findAttachment = [](const std::vector<RenderTextureResouce*>& resouceList, const RenderTextureResouce* resource) -> bool
	{
		if (!resource)
			return false;

		auto itr = std::find_if(resouceList.begin(), resouceList.end(), [resource](const RenderTextureResouce* res)
			{
				return res->GetPhysicalIndex() == resource->GetPhysicalIndex();
			});
		return itr != resouceList.end();
	};

	//const auto findBuffer = [](const std::vector<RenderBufferResource*>& resouceList, const RenderBufferResource* resource) -> bool
	//{
	//	if (!resource)
	//		return false;

	//	auto itr = std::find_if(resouceList.begin(), resouceList.end(), [resource](const RenderBufferResource* res)
	//		{
	//			return res->GetPhysicalIndex() == resource->GetPhysicalIndex();
	//		});
	//	return itr != resouceList.end();
	//};

	const auto shoudMerge = [&](const RenderPass& prev, const RenderPass& next) -> bool
	{
		// Different queues, don't merge.
		if ((prev.GetQueue() & ComputeQueues) || next.GetQueue() != prev.GetQueue())
			return false;

		//Check if we should allow subpass merging
		if (!(bool)CONFIG_VAL(Config::GraphicsConfig.MergeSubpasses))
			return false;

		// Check for mip mapping. If we need to mip map don't merge.
		for (auto* output : prev.GetColorOutputs())
		{
			if (m_physicalResources.Dimensions[output->GetPhysicalIndex()].Levels > 1)
				return false;
		}

		// Need non-local dependency.
		for (auto& input : next.GetGenericTextureInputs())
		{
			if (findAttachment(prev.GetColorOutputs(), input.Texture))
				return false;
			/*			if (findAttachment(prev.GetResolveOutputs(), input.Texture))
							return false;
						if (findAttachment(prev.GetStoreageTextureOutputs(), input.Texture))
							return false;
						if (findAttachment(prev.GetBlitTextureOutputs(), input.Texture))
							return false;	*/
			if (input.Texture && prev.GetDepthStencilOutput() == input.Texture)
				return false;
		}

		// Need non-local dependency, cannot merge.
		//for (auto* input : next.GetGenericBufferInputs())
		//{
		//	if (findBuffer(prev.GetStorageOutputs(), input.Buffer))
		//		return false;
		//}

		// Need non-local dependency, cannot merge.
		//for (auto* input : next.GetStorageInputs())
		//{
		//	if (findBuffer(prev.GetStorageInputs(), input.Buffer))
		//		return false;
		//}

		// Need non-local dependency, cannot merge.
		//for (auto* input : next.GetBlitTextureInputs())
		//{
		//	if (findAttachment(prev.GetBlitTextureInputs(), input.Texture))
		//		return false;
		//}

		// Need non-local dependency, cannot merge.
		//for (auto* input : next.GetStorageTextureInputs())
		//{
		//	if (findAttachment(prev.GetStorageTextureInputs(), input.Texture))
		//		return false;
		//}

		const auto differentAttachment = [](const RenderGraphResource* a, const RenderGraphResource* b)
		{
			return a && b && a->GetPhysicalIndex() != b->GetPhysicalIndex();
		};
		const auto sameAttachment = [](const RenderGraphResource* a, const RenderGraphResource* b)
		{
			return a && b && a->GetPhysicalIndex() == b->GetPhysicalIndex();
		};

		// Need a different depth attachment, break up the pass.
		if (differentAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilInput()))
			return false;
		if (differentAttachment(next.GetDepthStencilOutput(), prev.GetDepthStencilInput()))
			return false;
		if (differentAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilOutput()))
			return false;
		if (differentAttachment(next.GetDepthStencilOutput(), prev.GetDepthStencilOutput()))
			return false;

		for (auto* input : next.GetColorInputs())
		{
			if (!input)
				continue;
			//if (findAttachment(prev.GetStorageTextureOutputs(), input)
			//	return false;
			//if (findAttachment(prev.GetBlitTextureOutputs(), input)
			//	return false;
		}

		// Now, we have found all failure cases, try to see if we *should* merge.

		// Keep color on tile.
		for (auto* input : next.GetColorInputs())
		{
			if (!input)
				continue;
			if (findAttachment(prev.GetColorOutputs(), input))
				return true;
			//if (findAttachment(prev.GetResolveTextureOutputs(), input))
			//	return true;
		}

		// Keep depth on tile.
		if (sameAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilInput()) ||
			sameAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilOutput()))
		{
			return true;
		}

		// Keep depth attachment or color on tile.
		for (auto* input : next.GetAttachmentInputs())
		{
			if (findAttachment(prev.GetColorOutputs(), input))
				return true;
			//if (findAttachment(prev.GetResolveTextureOutputs(), input))
			//	return true;
			if (input && prev.GetDepthStencilOutput() == input)
				return true;
		}

		// No reason to merge, so don't.
		return false;
	};

	for (U32 index = 0; index < m_passStack.size();)
	{
		U32 mergeEnd = index + 1;
		for (; mergeEnd < m_passStack.size(); ++mergeEnd)
		{
			bool merge = true;
			for (U32 mergeStart = index; mergeStart < mergeEnd; ++mergeStart)
			{
				if (!shoudMerge(*m_passes[m_passStack[mergeStart]], *m_passes[m_passStack[mergeEnd]]))
				{
					merge = false;
					break;
				}
			}
			if (!merge)
				break;
		}

		physicalPass.Passes.insert(physicalPass.Passes.end(), m_passStack.begin() + index, m_passStack.begin() + mergeEnd);
		m_physicalPasses.push_back(physicalPass);
		index = mergeEnd;
	}

	for (auto& physPass : m_physicalPasses)
	{
		U32 index = U32(&physPass - m_physicalPasses.data());
		for (auto& pass : physPass.Passes)
		{
			m_passes[pass]->SetPhysicalPassIndex(index);
		}
	}
}