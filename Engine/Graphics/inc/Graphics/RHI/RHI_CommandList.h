#pragma once

#include "Graphics/RHI/RHI_Resource.h"
#include "Graphics/RHI/RHI_Renderpass.h"
#include "Graphics/RHI/RHI_Buffer.h"

#include "Graphics/PipelineStateObject.h"
#include "Graphics/PipelineBarrier.h"

#include "Core/Delegate.h"

#include <unordered_set>
#include <glm/vec2.hpp>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		class RHI_Texture;
		class RHI_Sampler;
		class RHI_DescriptorLayout;
		class RHI_CommandListAllocator;

		class RenderContext;
		class RenderGraph;
		class DescriptorAllocator;

		enum class RHI_CommandListStates : uint8_t
		{
			Idle,
			Recording,
			Ended,
			Submitted
		};

		/// Store relevant draw data.
		struct RHI_CommandListCurrentDrawData
		{
			glm::vec2 Viewport;
			glm::ivec2 Siccsior;

			RHI_Buffer* VertexBuffer = nullptr;
			RHI_Buffer* IndexBuffer = nullptr;
		};

		class IS_GRAPHICS RHI_CommandList : public RHI_Resource
		{
		public:

			static RHI_CommandList* New();

			virtual void Reset();

			virtual void Close() = 0;

			virtual void Create(RenderContext* context) = 0;
			virtual void PipelineBarrier(PipelineBarrier barrier) = 0;

			virtual void CopyBufferToBuffer(RHI_Buffer* dst, u64 dstOffset, RHI_Buffer* src, u64 srcOffset, u64 sizeInBytes) = 0;
			void CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src);

			void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src) { CopyBufferToImage(dst, src, 0); }
			virtual void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src, u64 offset) = 0;

			void SetImageLayout(RHI_Texture* texture, ImageLayout layout);

			virtual void BeginRenderpass(RenderpassDescription renderDescription) = 0;
			virtual void EndRenderpass() = 0;

			virtual void SetPipeline(PipelineStateObject pso) = 0;

			virtual void SetPushConstant(u32 offset, u32 size, const void* data) = 0;

			void SetUniform(u32 set, u32 binding, const void* data, u32 size);

			template<typename T>
			void SetUniform(u32 set, u32 binding, const T& data) { SetUniform(set, binding, static_cast<const void*>(&data), sizeof(T)); }
			virtual void SetTexture(u32 set, u32 binding, const RHI_Texture* texture);
			virtual void SetTexture(u32 set, u32 binding, const RHI_Texture* texture, const RHI_Sampler* sampler);
			virtual void SetSampler(u32 set, u32 binding, const RHI_Sampler* sampler);

			/// @brief Upload data to the frame uniform buffer. Does not bind.
			/// @param set 
			/// @param binding 
			/// @param data 
			/// @param size 
			/// @return RHI_BufferView
			RHI_BufferView UploadUniform(const void* data, u32 size);
			template<typename T>
			RHI_BufferView UploadUniform(const T& data) { return UploadUniform(static_cast<const void*>(&data), sizeof(T)); }

			/// @brief Binds a uploaded buffer to a uniform.
			/// @param set 
			/// @param binding 
			/// @param buffer 
			virtual void SetUniform(u32 set, u32 binding, RHI_BufferView buffer);

			virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y = false) = 0;
			virtual void SetScissor(int x, int y, int width, int height) = 0;
			virtual void SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor) = 0;
			virtual void SetLineWidth(float width) = 0;

			virtual void SetVertexBuffer(RHI_Buffer* buffer) = 0;
			virtual void SetIndexBuffer(RHI_Buffer* buffer, IndexType index_type) = 0;

			virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) = 0;
			virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) = 0;

			virtual void BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout) = 0;

			virtual void BeginTimeBlock(const std::string& blockName);
			virtual void BeginTimeBlock(const std::string& blockName, glm::vec4 colour);
			virtual void EndTimeBlock();

			bool IsDiscard() const { return m_discard; }

			/// <summary>
			/// This delegate is called when this command buffer has completed all work on the GPU.
			/// </summary>
			Core::Delegate<> OnWorkCompleted;

		protected:
			bool CanDraw();
			virtual bool BindDescriptorSets();

			virtual void SetImageLayoutTransition(RHI_Texture* texture, ImageLayout layout) = 0;

			RenderContext* m_context{ nullptr };
			bool m_activeRenderpass = false;
			/// @brief Discard this command list. Something has happened which no longer allows us to use this command list this frame.
			bool m_discard = false;

			PipelineStateObject m_pso;
			PipelineStateObject m_activePSO;

			RHI_CommandListStates m_state = RHI_CommandListStates::Idle;

			RHI_CommandListCurrentDrawData m_drawData;

			DescriptorAllocator* m_descriptorAllocator = nullptr;

			RHI_Buffer* m_bound_vertex_buffer = nullptr;
			RHI_Buffer* m_bound_index_buffer = nullptr;


			friend class RenderGraph;
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
			std::mutex m_lock;
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

			void Reset();

		private:
			RenderContext* m_context = nullptr;
			RHI_CommandListAllocator* m_allocator = nullptr;
		};
	}
}