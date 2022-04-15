#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

namespace Insight
{
	Graphics::CommandList Renderer::s_FrameCommandList;
	Graphics::RenderContext* Renderer::s_context;
	
	namespace Graphics
	{
		RenderContext* RenderContext::New()
		{
			RenderContext* context = nullptr;
			if (GraphicsManager::IsVulkan()) { context = new RHI::Vulkan::RenderContext_Vulkan(); }
			else if (GraphicsManager::IsDX12()) { context = nullptr; }
			
			::Insight::Renderer::s_context = context;
			context->m_shaderManager.SetRenderContext(context);

			return context;
		}
	}

	
	// Renderer
	Graphics::GPUBuffer* Renderer::CreateVertexBuffer(u64 sizeBytes)
	{
		return s_context->CreateVertexBuffer(sizeBytes);
	}

	Graphics::GPUBuffer* Renderer::CreateIndexBuffer(u64 sizeBytes)
	{
		return s_context->CreateIndexBuffer(sizeBytes);
	}

	void Renderer::FreeVertexBuffer(Graphics::GPUBuffer* buffer)
	{
		s_context->FreeVertexBuffer(buffer);
	}

	void Renderer::FreeIndexBuffer(Graphics::GPUBuffer* buffer)
	{
		s_context->FreeIndexBuffer(buffer);
	}

	void Renderer::BindVertexBuffer(Graphics::GPUBuffer* buffer)
	{
		s_FrameCommandList.SetVertexBuffer(buffer);
	}

	void Renderer::BindIndexBuffer(Graphics::GPUBuffer* buffer)
	{
		s_FrameCommandList.SetIndexBuffer(buffer);
	}

	Graphics::RHI_Shader* Renderer::GetShader(Graphics::ShaderDesc desc)
	{
		return s_context->m_shaderManager.GetOrCreateShader(desc);
	}

	void Renderer::SetPipelineStateObject(Graphics::PipelineStateObject pso)
	{
		s_FrameCommandList.SetPipelineStateObject(pso);
	}

	void Renderer::SetViewport(int width, int height)
	{
		s_FrameCommandList.SetViewport(width, height);
	}

	void Renderer::SetScissor(int width, int height)
	{
		s_FrameCommandList.SetScissor(width, height);
	}

	void Renderer::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		s_FrameCommandList.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Renderer::DrawIndexed()
	{
		s_FrameCommandList.DrawIndexed(0, 0, 0, 0, 0);
	}
}