#pragma once

#include "Graphics/CommandList.h"
#include "Graphics/RHI/RHI_Resource.h"
#include <unordered_set>

#include <glm/vec2.hpp>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		struct FrameResouce;

		class RHI_Texture;

		// Store relevant draw data.
		struct RHI_CommandListCurrentDrawData
		{
			glm::vec2 Viewport;
			glm::ivec2 Siccsior;

			RHI_Buffer* VertexBuffer = nullptr;
			RHI_Buffer* IndexBuffer = nullptr;
		};

		class RHI_CommandList : public RHI_Resource
		{
		public:

			static RHI_CommandList* New();

			void Record(CommandList& cmdList, FrameResouce* frameResouces);
			virtual void Reset();

			virtual void Close() = 0;

			virtual void CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset) = 0;
			void CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src) { CopyBufferToBuffer(dst, src, 0); }

			virtual void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src) = 0;

			virtual void BeginRenderpass() = 0;
			virtual void EndRenderpass() = 0;

			virtual void SetPipeline(PipelineStateObject pso) = 0;
			virtual void SetUniform(int set, int binding, DescriptorBufferView view) = 0;
			virtual void SetTexture(int set, int binding, RHI_Texture* texture) = 0;
			virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth) = 0;
			virtual void SetScissor(int x, int y, int width, int height) = 0;

			virtual void SetVertexBuffer(RHI_Buffer* buffer) = 0;
			virtual void SetIndexBuffer(RHI_Buffer* buffer) = 0;

			virtual void Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance) = 0;
			virtual void DrawIndexed(int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance) = 0;

			virtual void BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout) = 0;

		protected:
			bool CanDraw();
			virtual bool BindDescriptorSets();

			RenderContext* m_context{ nullptr };
			FrameResouce* m_frameResouces;
			bool m_activeRenderpass = false;
			PipelineStateObject m_pso;
			PipelineStateObject m_activePSO;

			RHI_CommandListCurrentDrawData m_drawData;
		};

		class RHI_CommandListAllocator : public RHI_Resource
		{
		public:

			static RHI_CommandListAllocator* New();

			virtual void Create(RenderContext* context) = 0;
			
			virtual void Reset() = 0;

			virtual RHI_CommandList* GetCommandList() = 0;
			virtual RHI_CommandList* GetSingleSubmitCommandList() = 0;

			void ReturnCommandList(RHI_CommandList* cmdList);
			virtual void ReturnSingleSubmitCommandList(RHI_CommandList* cmdList) = 0;

		protected:
			std::unordered_set<RHI_CommandList*> m_allocLists;
			std::unordered_set<RHI_CommandList*> m_freeLists;
		};

		class CommandListManager
		{
		public:
			CommandListManager();
			~CommandListManager();

			void Create(RenderContext* context);
			void Update();
			void Destroy();

			RHI_CommandList* GetCommandList();
			RHI_CommandList* GetSingleUseCommandList();
			void ReturnCommandList(RHI_CommandList* cmdList);
			void ReturnSingleUseCommandList(RHI_CommandList* cmdList);

		private:
			RenderContext* m_context = nullptr;
			RHI_CommandListAllocator* m_allocator = nullptr;
		};
	}
}