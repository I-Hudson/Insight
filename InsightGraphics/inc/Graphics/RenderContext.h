#pragma once

#include "Graphics/Defines.h"
#include "Graphics/CommandList.h"
#include "Graphics/Enums.h"
#include "imgui.h"

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/RHI_Shader.h"

#include "Core/Collections/FactoryMap.h"

namespace Insight
{
	class Renderer;
	
	namespace Graphics
	{
		class RenderContext
		{
		public:

			static RenderContext* New();

			virtual bool Init() = 0;
			virtual void Destroy() = 0;

			virtual void InitImGui() = 0;
			virtual void DestroyImGui() = 0;

			virtual void Render(CommandList cmdList) = 0;

		protected:
			void ImGuiBeginFrame();
			void ImGuiRender();

			virtual void WaitForGpu() = 0;

		private:
			virtual RHI_Buffer* CreateVertexBuffer(u64 sizeBytes) = 0;
			virtual RHI_Buffer* CreateIndexBuffer(u64 sizeBytes) = 0;
			virtual void FreeVertexBuffer(RHI_Buffer* buffer) = 0;
			virtual void FreeIndexBuffer(RHI_Buffer* buffer) = 0;

		protected:
			const static int c_FrameCount = 2;

			::Core::FactoryMap<RHI_Buffer*> m_vertexBuffer;
			RHI_ShaderManager m_shaderManager;

			friend class Renderer;
		};
	}

	// Utility class for all other engine systems to call into.
	class IS_GRAPHICS Renderer
	{
	public:
		static void SetImGUIContext(ImGuiContext*& context);

		static Graphics::RHI_Buffer* CreateVertexBuffer(u64 sizeBytes);
		static Graphics::RHI_Buffer* CreateIndexBuffer(u64 sizeBytes);

		static void FreeVertexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeIndexBuffer(Graphics::RHI_Buffer* buffer);

		static void BindVertexBuffer(Graphics::RHI_Buffer* buffer);
		static void BindIndexBuffer(Graphics::RHI_Buffer* buffer);

		static Graphics::RHI_Shader* GetShader(Graphics::ShaderDesc desc);

		static void SetPipelineStateObject(Graphics::PipelineStateObject pso);

		static void SetViewport(int width, int height);
		static void SetScissor(int width, int height);

		static void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
		static void DrawIndexed();

		static Graphics::CommandList s_FrameCommandList;

	private:
		static Graphics::RenderContext* s_context;
		friend class Graphics::RenderContext;
	};
}