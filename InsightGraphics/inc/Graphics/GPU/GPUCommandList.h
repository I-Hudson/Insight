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

		class GPUCommandList
		{
		public:
			GPUCommandList() { }
			virtual ~GPUCommandList() { }

			bool IsEmpty() const { return m_recordCommandCount == 0; }
			u32 GetRecordCommandCount() const { return m_recordCommandCount; }
			GPUQueue GetQueue() const { return m_queue; }
			GPUCommandListType GetType() const { return m_type; }

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
			void SetSwapchainSubmit(bool swapchainSubmit);

			bool CanDraw();
			void Reset();

		protected:
			GPUCommandListState m_state;
			u32 m_recordCommandCount = 0;
			GPUQueue m_queue;
			GPUCommandListType m_type;
			PipelineStateObject m_pso;

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

		protected:
			GPUQueue m_queue = GPUQueue::GPUQueue_Graphics;
			std::list<GPUCommandList*> m_allocatedCommandLists;
		};


		class GPUCommandListManager
		{
		public:
			GPUCommandListManager();
			~GPUCommandListManager();

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
			GPUComamndListAllocator* m_commandListAllocator{ nullptr };
		};
	}
}