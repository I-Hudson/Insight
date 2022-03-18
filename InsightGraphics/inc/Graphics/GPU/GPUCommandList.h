#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/GPU/GPUPipelineStateObject.h"
#include <vector>
#include <list>

namespace Insight
{
	namespace Graphics
	{
		class GPUSemaphore;
		class GPUFence;
		class GPUShader;
		class GPUBuffer;

		class GPUCommandListManager;

		enum class GPUCommandListType
		{
			Default,
			Transient,
			Reset
		};

		enum class GPUCommandListState
		{
			Idle,
			Recording,
			Submitted
		};

		struct GPUCommandListActive
		{
			bool Renderpass = false;
			PipelineStateObject ActivePso;
			GPUBuffer* VertexBuffer = nullptr;
			GPUBuffer* IndexBuffer = nullptr;
		};

		class GPUCommandList
		{
		public:
			GPUCommandList() { }
			virtual ~GPUCommandList() { }

			bool IsEmpty() const { return m_recordCommandCount == 0; }
			u32 GetRecordCommandCount() const { return m_recordCommandCount; }
			GPUQueue GetQueue() const { return m_queue; }
			GPUCommandListType GetType() const { return m_type; }

			void CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst);
			virtual void CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, u64 srcOffset, u64 dstOffset, u64 size) = 0;

			virtual void SetViewport(int width, int height) = 0;
			virtual void SetScissor(int width, int height) = 0;

			virtual void SetVertexBuffer(GPUBuffer* buffer) = 0;
			virtual void SetIndexBuffer(GPUBuffer* buffer) = 0;

			virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) = 0;
			virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) = 0;

			void Submit(GPUQueue queue);
			void SubmitAndWait(GPUQueue queue);
			void SubmitAndWait(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence);
			virtual void Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence) = 0;

			void SetShader(GPUShader* shader);
			void AddRenderTarget(RenderTarget* renderTarget);
			void ClearRenderTargets();
			void SetPrimitiveTopologyType(PrimitiveTopologyType primitiveTopologyType);
			void SetPolygonMode(PolygonMode polygonMode);
			void SetCullMode(CullMode cullMode);
			void SetFrontFace(FrontFace frontFace);
			void SetSwapchainSubmit(bool swapchainSubmit);

			virtual void BeginRecord() = 0;
			virtual void EndRecord() = 0;

			virtual void BeginRenderpass() = 0;
			virtual void EndRenderpass() = 0;

			virtual void BindPipeline(GPUPipelineStateObject* pipeline) = 0;

			bool CanDraw();
			void Reset();

		protected:
			GPUCommandListState m_state = GPUCommandListState::Idle;
			u32 m_recordCommandCount = 0;
			GPUQueue m_queue;
			GPUCommandListType m_type;
			PipelineStateObject m_pso;
			GPUCommandListActive m_activeItems;

			friend class GPUCommandListManager;
		};

		class GPUComamndListAllocator
		{
		public:
			virtual ~GPUComamndListAllocator() { }
			
			void SetQueue(GPUQueue queue) { m_queue = queue; }

			virtual GPUCommandList* AllocateCommandList(GPUCommandListType type) = 0;

			virtual void ResetCommandLists(std::list<GPUCommandList*> cmdLists) = 0;
			virtual void ResetCommandPool(GPUCommandListType type) = 0;

			virtual void FreeCommandList(GPUCommandList* cmdList) = 0;
			virtual void FreeCommandLists(const std::list<GPUCommandList*>& cmdLists) = 0;
			virtual void FreeAllCommandLists() = 0;

			virtual void Destroy() = 0;

		protected:
			GPUQueue m_queue = GPUQueue::GPUQueue_Graphics;
			std::list<GPUCommandList*> m_allocatedCommandLists;
		};


		class GPUCommandListManager
		{
		public:
			GPUCommandListManager();
			~GPUCommandListManager();

			static GPUCommandListManager& Instance()
			{
				static GPUCommandListManager ins;
				return ins;
			}

			void Create();
			void SetQueue(GPUQueue queue);

			void ResetCommandList(GPUCommandList* cmdList);
			void ResetCommandLists(std::list<GPUCommandList*> cmdLists);

			void ResetCommandPool(GPUCommandListType type = GPUCommandListType::Default);

			GPUCommandList* GetOrCreateCommandList(GPUCommandListType type = GPUCommandListType::Default);
			void ReturnCommandList(GPUCommandList* cmdList);
			void Destroy();

		private:
			std::list<GPUCommandList*> m_inUseCommandLists;
			std::list<GPUCommandList*> m_freeCommandLists;
			std::map<GPUCommandListType, std::vector<GPUCommandList*>> m_typeToListLookup;
			GPUComamndListAllocator* m_commandListAllocator{ nullptr };
		};
	}
}