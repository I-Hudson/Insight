#pragma once

#include "Graphics/RHI/RHI_Resource.h"

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_Buffer;

		class RHI_BufferView
		{
		public:
			RHI_BufferView();
			RHI_BufferView(RHI_Buffer* buffer, int offset, int size);
			RHI_BufferView(const RHI_BufferView& other);
			RHI_BufferView(RHI_BufferView&& other);

			bool IsValid() const { return m_buffer; }

		private:
			RHI_Buffer* m_buffer = nullptr;
			int m_offset = 0;
			int m_size = 0;
		};

		class RHI_Buffer : public RHI_Resource
		{
		public:
			static RHI_Buffer* New();

			virtual void Create(RenderContext* context, BufferType bufferType, u64 sizeBytes) = 0;
			virtual RHI_BufferView Upload(void* data, int sizeInBytes, int offset) = 0;

			virtual RHI_BufferView Upload(void* data, int sizeInBytes) { return Upload(data, sizeInBytes, 0); }
			
			u64 GetSize() const { return m_size; }
			BufferType GetType() const { return m_bufferType; }

		protected:
			virtual void Resize(int newSizeInBytes) = 0;

		protected:
			BufferType m_bufferType;
			u64 m_size = 0;

			friend class RenderContext;
			friend class RHI_DynamicBuffer;
		};

		class RHI_DynamicBuffer
		{
		public:
			void Create(RenderContext* context);
			void Release();

			RHI_BufferView Upload(void* data, int sizeInBytes);

			void Resize(int newSizeInBytes);
			void Reset();

		private:
			RenderContext* m_context = nullptr;
			RHI_Buffer* m_buffer = nullptr;
			int m_size = 0;
			int m_capacity = 0;
		};
	}
}