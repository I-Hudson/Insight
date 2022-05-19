#pragma once

#include "Graphics/Defines.h"
#include "Graphics/CommandList.h"
#include "Graphics/Enums.h"
#include "imgui.h"

#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/RHI/RHI_Descriptor.h"

#include "Core/Collections/FactoryMap.h"

namespace Insight
{
	class Renderer;
	
	namespace Graphics
	{
		class RenderContext;
		class RHI_Texture;

		class RenderContext
		{
		public:
			static RenderContext* New();

			virtual bool Init() = 0;
			virtual void Destroy() = 0;

			virtual void InitImGui() = 0;
			virtual void DestroyImGui() = 0;

			virtual void Render(CommandList cmdList) = 0;

			virtual void GpuWaitForIdle() = 0;
			virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) = 0;

			bool HasExtension(DeviceExtension extension);

			RHI_DescriptorLayoutManager& GetDescriptorLayoutManager() { return m_descriptorLayoutManager; }

		protected:
			void ImGuiBeginFrame();
			void ImGuiRender();

			virtual void WaitForGpu() = 0;

			void BaseDestroy();

		private:
			RHI_Buffer* CreateBuffer(BufferType bufferType, u64 sizeBytes, int stride);
			void FreeBuffer(RHI_Buffer* buffer);
			int GetBufferCount(BufferType bufferType) const;

		protected:
			const static int c_FrameCount = 3;

			std::array<u8, (size_t)DeviceExtension::DeviceExtensionCount> m_deviceExtensions;

			RHI_ResourceManager<RHI_Buffer> m_vertexBuffer;
			std::map<BufferType, RHI_ResourceManager<RHI_Buffer>> m_buffers;
			RHI_ShaderManager m_shaderManager;

			RHI_DescriptorLayoutManager m_descriptorLayoutManager;

			friend class Renderer;
		};

		struct FrameResouce
		{
			RHI_DynamicBuffer UniformBuffer;
			CommandListManager CommandListManager;

			virtual void Reset();
		};
	}

	// Utility class for all other engine systems to call into.
	class IS_GRAPHICS Renderer
	{
	public:
		static void SetImGUIContext(ImGuiContext*& context);

		static Graphics::RHI_Buffer* CreateVertexBuffer(u64 sizeBytes, int stride);
		static Graphics::RHI_Buffer* CreateIndexBuffer(u64 sizeBytes);
		static Graphics::RHI_Buffer* CreateUniformBuffer(u64 sizeBytes);
		static Graphics::RHI_Buffer* CreateRawBuffer(u64 sizeBytes);

		static void FreeVertexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeIndexBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeUniformBuffer(Graphics::RHI_Buffer* buffer);
		static void FreeRawBuffer(Graphics::RHI_Buffer* buffer);

		static int GetVertexBufferCount();
		static int GetIndexBufferCount();
		static int GetUniformBufferCount();
		static int GetBufferCount(Graphics::BufferType bufferType);

		static void BindVertexBuffer(Graphics::RHI_Buffer* buffer);
		static void BindIndexBuffer(Graphics::RHI_Buffer* buffer);

		static Graphics::RHI_Shader* GetShader(Graphics::ShaderDesc desc);

		static void SetPipelineStateObject(Graphics::PipelineStateObject pso);

		static void SetViewport(int width, int height);
		static void SetScissor(int width, int height);

		static void SetUniform(int set, int binding, void* data, int sizeInBytes);
		static void SetTexture(int set, int binding, Graphics::RHI_Texture* texture);

		static void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
		static void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance);

		static Graphics::CommandList s_FrameCommandList;

	private:
		static Graphics::RenderContext* s_context;
		friend class Graphics::RenderContext;
	};
}