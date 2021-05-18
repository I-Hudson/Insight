#include "ispch.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include "Engine/Config/Config.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/Graphics/PixelFormatExtensions.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include <map>

namespace Insight::Graphics
{
	RenderGraph* RenderGraph::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::RenderGraphVulkan>();
		}	
		ASSERT(false && "[RenderGraph::New] API implementation is missing.")
			return nullptr;
	}

	RenderGraph::RenderGraph()
		: m_built(false)
		, m_changed(false)
		, m_frameIndex(0)
		, m_swapchainPresentPass(RenderPass(this, -1, "SwapchainPresentPass", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT))
	{
		m_swapchain = GPUSwapchain::New();
		m_swapchain->Init();
		m_swapchain->Build(Graphics::GPUSwapchainDesc());

		for (u32 i = 0; i < m_swapchain->GetImageCount(); ++i)
		{
			m_swapchainSubmision.push_back(SwapchainSubmision());
			SwapchainSubmision& sub = m_swapchainSubmision.back();

			sub.Image = m_swapchain->GetImage(i);
			sub.GraphPass = GPURenderGraphPass::New();
			sub.GraphPass->InitForSwapchain(m_swapchain, sub.Image);
			sub.GraphPass->m_graph = this;

			// Swapchain resources needed for submission.
			sub.Shader = GPUShader::New();
			sub.Shader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/present.vert", ShaderStageInput::FilePath);
			sub.Shader->SetStage(ShaderStage::Fragment, "./data/shaders/vulkan/present.frag", ShaderStageInput::FilePath);
			sub.Shader->Compile();

			sub.Pipeline = GPUPipeline::New();
			sub.Pipeline->SetShader(sub.Shader);
			sub.Pipeline->Init(sub.GraphPass, Graphics::GPUPipelineDesc(PrimitiveTopologyType::Triangle_List, PolygonMode::Fill, CullMode::Back, FrontFace::Counter_Clockwise));
		}

		m_swapchainPresentPass.SetClearColour(glm::vec4(0.9f, 0.2f, 0.7f, 1.0f));
		m_swapchainPresentPass.SetRenderFunc([this](GPUCommandBuffer* cmdBuffer, FrameBufferResources& buffers, GPUDescriptorBuilder* builder, RenderPass& pass)
		{
			IS_PROFILE_SCOPE("Swapchain present pass.");
			const SwapchainSubmision& swapchain = RenderGraph::Instance()->GetCurrentSwapchainSubmision();

			cmdBuffer->BindPipeline(PipelineBindPoint::Graphics, swapchain.Pipeline);

			GPUImage* sampleImage = m_physicalImages.at(m_resources.at(m_resourceToIndex.at(m_backBufferSource)).GetPhysicalIndex());
			Graphics::GPUDescriptorSet* samplerSet = Graphics::GPUDescriptorSet::New();
			builder->BindImage(0, sampleImage, DescriptorType::Combined_Image_Sampler, ShaderStage::Fragment)->Build(samplerSet);

			Graphics::GPUDescriptorSet* sets[] = { samplerSet };
			cmdBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, swapchain.Pipeline, 0, ARRAY_COUNT(sets), sets, 0, nullptr);

			cmdBuffer->Draw(3, 1, 0, 0);

			::Delete(samplerSet);
		});

		m_frames.resize(c_MaxFrameCount);
	}

	RenderGraph::~RenderGraph()
	{
		//m_singleFrame.ReleaseGPU();
		for (auto& frame : m_frames)
		{
			frame.ReleaseGPU();
		}

		for (auto& swapchainImage : m_swapchainSubmision)
		{
			::Delete(swapchainImage.GraphPass);
			swapchainImage.Shader->ReleaseGPU();
			::Delete(swapchainImage.Shader);
			swapchainImage.Pipeline->ReleaseGPU();
			::Delete(swapchainImage.Pipeline);
		}

		m_swapchain->ReleaseGPU();
		::Delete(m_swapchain);

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
			u32 newIndex = (u32)m_passes.size();
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
		IS_PROFILE_FUNCTION();

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
		IS_PROFILE_FUNCTION();

		auto& frame = m_frames.at(m_frameIndex);
		frame.Init();
		frame.Passes = m_passes;
		frame.PassStack = m_passStack;

		{
			IS_PROFILE_SCOPE("Get next swapchain image.");
			GPUResults res = m_swapchain->GetNextImage(frame.SwapchainImageAquired, &m_swapchainImageIndex);
			if (res == GPUResults::Error_Out_Of_Data)
			{
				SwapchainRebuild();
				return;
			}
		}

		auto* cmdBuffer = frame.CommandBuffers;
		{
			IS_PROFILE_SCOPE("Record render passes.");
			cmdBuffer->BeginRecord();
			for (auto& passIndex : frame.PassStack)
			{
				auto& pass = frame.Passes.at(passIndex);
				auto* renderPass = frame.RenderPasses.at(pass.GetColorOutputHash());
				pass.CallBeginRenderFunc(renderPass);
				cmdBuffer->BeginRenderpass(renderPass);
				Maths::Rect viewPortRect = Maths::Rect(0, pass.GetWindowRect().GetHeight(), pass.GetWindowRect().GetWidth(), -pass.GetWindowRect().GetHeight());
				cmdBuffer->SetViewPort(viewPortRect);
				cmdBuffer->SetScissor(pass.GetWindowRect());
				pass.CallRenderFunc(cmdBuffer, frame.Buffers, frame.DescriptorBuilder);
				cmdBuffer->EndRenderpass(renderPass);
				pass.CallEndRenderFunc();
			}
		}

		{
			IS_PROFILE_SCOPE("Render to swapchain image.");
			auto& swapchainSubmision = m_swapchainSubmision.at(m_swapchainImageIndex);
			m_swapchainPresentPass.CallBeginRenderFunc(swapchainSubmision.GraphPass);
			cmdBuffer->BeginRenderpass(swapchainSubmision.GraphPass);
			cmdBuffer->SetViewPort(m_swapchainPresentPass.GetWindowRect());
			cmdBuffer->SetScissor(m_swapchainPresentPass.GetWindowRect());
			m_swapchainPresentPass.CallRenderFunc(cmdBuffer, frame.Buffers, frame.DescriptorBuilder);
			cmdBuffer->EndRenderpass(swapchainSubmision.GraphPass);
			m_swapchainPresentPass.CallEndRenderFunc();
		}

		{
			IS_PROFILE_SCOPE("End record and submit go gpu.");
			cmdBuffer->EndRecord();
			cmdBuffer->Submit(GPUQueue::GRAPHICS, frame.Fence);
		}

		{
			IS_PROFILE_SCOPE("Present to swapchain.");
			std::vector<GPUSemaphore*> waitSemaphores = { frame.SwapchainImageAquired };
			GPUResults res = m_swapchain->Present(GPUQueue::GRAPHICS, m_swapchainImageIndex, waitSemaphores);
			if (res == GPUResults::Error_Out_Of_Data)
			{
				SwapchainRebuild();
				return;
			}
		}
		m_frameIndex = (m_frameIndex + 1) % c_MaxFrameCount;
	}

	void RenderGraph::Reset()
	{
		IS_PROFILE_FUNCTION();
		
		auto& frame = m_frames.at(m_frameIndex);

		m_passes.clear();
		m_passToIndex.clear();
		m_passStack.clear();
		if (frame.Initialised)
		{
			{
				IS_PROFILE_SCOPE("[RenderGraph::Reset] single frame wait/reset");
				frame.Fence->Wait();
				frame.Fence->Reset();
			}
			frame.Reset();
		}
	}

	void RenderGraph::LogToConsole()
	{
		for (auto& renderPassIndex : m_passStack)
		{
			auto& renderPass = m_passes[renderPassIndex];
			IS_CORE_INFO("RenderPass: {0}", renderPass.GetPassName());
			IS_CORE_INFO("	Colour Inputs:");
			for (auto& inputIndex : renderPass.GetColorInputs() )
			{
				auto& input = m_resources[inputIndex];
				IS_CORE_INFO("		Name: {0}", input.GetName());
			}
			if (renderPass.IsDepthSencilInputValid())
			{
				IS_CORE_INFO("	DepthStencil Input: Enabled");
			}
			IS_CORE_INFO("	DepthStencil Input: Disabled");


			IS_CORE_INFO("	Colour Output:");
			for (auto& outputIndex : renderPass.GetColorOutputs())
			{
				auto& output = m_resources[outputIndex];
				IS_CORE_INFO("		Name: {0}", output.GetName());
			}
			if (renderPass.IsDepthSencilOuputValid())
			{
				IS_CORE_INFO("	DepthStencil Output: Enabled");
			}
			IS_CORE_INFO("	DepthStencil Output: Disabled");
		}
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
			u32 newIndex = (u32)m_resources.size();
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
	//		u32 newIndex = (u32)m_resources.size();
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
		IS_PROFILE_FUNCTION();

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
		IS_PROFILE_FUNCTION();
		std::map<RenderPassQueue, std::vector<u32>> passesByQueue;
		for (auto& passIndex : flattenedPasses)
		{
			auto& pass = m_passes.at(passIndex);
			passesByQueue[pass.m_passQueue].push_back(pass.GetPassIndex());
		}
		flattenedPasses.clear();

		for (auto& queue : passesByQueue)
		{
			// We now have all the passes and which other dependent passes.
			std::vector<u32> unorderedPasses;
			std::vector<u32> localFlattedPasses = queue.second;
			unorderedPasses.reserve(localFlattedPasses.size());
			std::swap(localFlattedPasses, unorderedPasses);

			const auto schedule = [&](unsigned index) {
				// Need to preserve the order of remaining elements.
				localFlattedPasses.push_back(unorderedPasses[index]);
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

				for (u32 i = 0; i < unorderedPasses.size(); ++i)
				{
					u32 overlapFactor = 0;
					for (auto itr = localFlattedPasses.rbegin(); itr != localFlattedPasses.rend(); ++itr)
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
						auto& pass = m_passes.at(queue.second.at(unorderedPasses.at(i)));
						auto& pass2 = m_passes.at(queue.second.at(unorderedPasses.at(j)));
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
			flattenedPasses.insert(flattenedPasses.end(), localFlattedPasses.begin(), localFlattedPasses.end());
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
		IS_PROFILE_FUNCTION();

		for (auto& resource : m_resources)
		{
			if (resource.GetPhysicalIndex() == RenderGraphResource::Unused)
			{
				u32 physicalIndex = (u32)m_physicalImages.size();

				GPUImage* image = GPUImage::New();
				GPUImageDesc desc = GPUImageDesc::Image2D(resource.TextureInfo.m_info.Width, resource.TextureInfo.m_info.Height, resource.TextureInfo.m_info.Depth, resource.TextureInfo.m_info.Levels,
														  resource.TextureInfo.m_info.Samples, resource.TextureInfo.m_info.Layers, ImageDomain::Physical, ImageLayout::Undefined, 
														  resource.TextureInfo.m_usageFlags, 0, 0, resource.TextureInfo.m_info.Format, ImageType::Image_2D, ImageUsageType::Render_Target, nullptr);
				desc.Sampler = resource.TextureInfo.m_info.SamplerDesc;

				image->Init(desc);
				image->SetName(resource.GetFullName());
				m_physicalImages.push_back(image);

				GPUImageView* imageView = GPUImageView::New();
				imageView->Init(GPUImageViewDesc(image, resource.TextureInfo.m_info.ViewInfo.ImageViewTytpe));
				m_physicalImageViews.push_back(imageView);

				resource.SetPhysicalIndex(physicalIndex);
			}
		}

		auto& frame = m_frames.at(m_frameIndex);
		for (auto& passIndex : m_passStack)
		{
			auto& pass = m_passes[passIndex];
			if (frame.RenderPasses.find(pass.GetColorOutputHash()) == frame.RenderPasses.end())
			{
				auto* frameBuffer = GPURenderGraphPass::New();
				frameBuffer->Init(pass);
				frame.RenderPasses.emplace(pass.GetColorOutputHash(), frameBuffer);
			}
		}
	}

	void RenderGraph::SwapchainRebuild()
	{
		while (Window::GetWidth() == 0 || Window::GetHeight() == 0)
		{
			Window::WaitForEvents();
		}

		GPUDevice::Instance()->WaitForGPU();

		auto& frame = m_frames.at(m_frameIndex);
		frame.Reset();

		for (auto& res : m_resources)
		{
			if (res.GetType() == RenderGraphResource::Type::Texture && !res.TextureInfo.m_info.AutoSizeToWindow)
			{
				continue;
			}

			if (res.GetType() == RenderGraphResource::Type::Texture)
			{
				// Texture should be recreated with the new window size.
				auto* physicalImage = m_physicalImages[res.GetPhysicalIndex()];
				Graphics::GPUImageDesc desc = physicalImage->GetDesc();
				desc.Width = Window::GetWidth();
				desc.Height = Window::GetHeight();
				physicalImage->Init(desc);

				auto* physicalImageView = m_physicalImageViews[res.GetPhysicalIndex()];
				physicalImageView->Init(physicalImage);
			}
		}

		for (auto& pass : m_passes)
		{
			pass.m_windowRect = Maths::Rect(0, 0, (float)Window::GetWidth(), (float)Window::GetHeight());
		}

		m_swapchain->Build(Graphics::GPUSwapchainDesc());
	}

	GPURenderGraphPass* GPURenderGraphPass::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPURenderGraphPassVulkan>();
		}
		ASSERT(false && "[RenderGraph::New] API implementation is missing.")
			return nullptr;
	}

	RenderPass& GPURenderGraphPass::GetRenderPass() const
	{
		if (m_swapchainPass)
		{
			return m_graph->m_swapchainPresentPass;
		}
		else
		{
			return m_graph->m_passes.at(m_renderPassIndex);
		}
	}

	void RenderGraph::FrameSubmision::Init()
	{
		if (Initialised)
		{
			return;
		}
		Initialised = true;

		CommandPools = GPUCommandPool::New();
		CommandPools->Init(GPUCommandPoolDesc::GPUCommandPoolDesc(GPUCommandPoolFlags::INVALID, GPUQueue::GRAPHICS));
		CommandBuffers = CommandPools->AllocateCommandBuffer(GPUCommandBufferDesc::CreateOneTimeCmdBuffer());

		Fence = GPUFence::New();
		Fence->Init(GPUFenceDesc());

		SwapchainImageAquired = GPUSemaphore::New();
		SwapchainImageAquired->Init(Graphics::GPUSemaphoreDesc());

		DescriptorAllocator = GPUDescriptorAllocator::New();
		DescriptorAllocator->Init();
		DescriptorLayoutCache = GPUDescriptorLayoutCache::New();

		DescriptorBuilder = GPUDescriptorBuilder::New();
		DescriptorBuilder->Begin(DescriptorLayoutCache, DescriptorAllocator);

		Buffers[GPUBufferFlags::VERTEX] = GPUDynamicBuffer::New();
		Buffers.at(GPUBufferFlags::VERTEX)->Init(GPUDynamicBufferDesc::Vertex(1 * 1024 * 1024));
		Buffers.at(GPUBufferFlags::VERTEX)->SetName("FrameDynamicBuffer " + std::to_string(RenderGraph::Instance()->GetFrameIndex()) + " Vertex");

		Buffers[GPUBufferFlags::INDEX] = GPUDynamicBuffer::New();
		Buffers.at(GPUBufferFlags::INDEX)->Init(GPUDynamicBufferDesc::Index(1 * 1024 * 1024));
		Buffers.at(GPUBufferFlags::INDEX)->SetName("FrameDynamicBuffer " + std::to_string(RenderGraph::Instance()->GetFrameIndex()) + " Index");

		Buffers[GPUBufferFlags::UNIFORM] = GPUDynamicBuffer::New();
		Buffers.at(GPUBufferFlags::UNIFORM)->Init(GPUDynamicBufferDesc::Uniform(64, 256, 1 * 1024 * 1024));
		Buffers.at(GPUBufferFlags::UNIFORM)->SetName("FrameDynamicBuffer " + std::to_string(RenderGraph::Instance()->GetFrameIndex()) + " Uniform");
	}

	void RenderGraph::FrameSubmision::Reset()
	{
		IS_PROFILE_FUNCTION();

		for (std::pair<u64, GPURenderGraphPass*> frameBuffer : RenderPasses)
		{
			::Delete(frameBuffer.second);
		}

		for (auto& buffer : Buffers)
		{
			buffer.second->Reset();
		}

		for (auto& pass : Passes)
		{
			pass.m_lifeTimeObjects.clear();
		}
		RenderPasses.clear();
		
		CommandPools->Reset();
		DescriptorAllocator->ResetPools();
	}

	void RenderGraph::FrameSubmision::ReleaseGPU()
	{
		GPUDevice::Instance()->WaitForGPU();
		Reset();
		CommandBuffers->ReleaseGPU();
		::Delete(CommandBuffers);
		CommandPools->ReleaseGPU();
		::Delete(CommandPools);
		Fence->ReleaseGPU();
		::Delete(Fence);
		SwapchainImageAquired->ReleaseGPU();
		::Delete(SwapchainImageAquired);
		::Delete(DescriptorBuilder);
		::Delete(DescriptorAllocator);
		::Delete(DescriptorLayoutCache);

		for (auto& buffer : Buffers)
		{
			buffer.second->ReleaseGPU();
			::Delete(buffer.second);
		}
	}
}