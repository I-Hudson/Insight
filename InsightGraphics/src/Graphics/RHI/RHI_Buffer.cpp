#include "Graphics/RHI/RHI_Buffer.h"
#include "Core/Memory.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/RenderContext.h"

#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_Buffer* RHI_Buffer::New()
		{
			if (GraphicsManager::IsVulkan()) { return nullptr; }
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_Buffer_DX12); }
			return nullptr;
		}


		RHI_BufferView::RHI_BufferView()
		{ }

		/// <summary>
		/// RHI_BufferView
		/// </summary>
		/// <param name="buffer"></param>
		/// <param name="offset"></param>
		/// <param name="size"></param>
		RHI_BufferView::RHI_BufferView(RHI_Buffer* buffer, int offset, int size)
			: m_buffer(buffer), m_offset(offset), m_size(size)
		{ }

		RHI_BufferView::RHI_BufferView(const RHI_BufferView& other)
		{
			m_buffer = other.m_buffer;
			m_offset = other.m_offset;
			m_size = other.m_size;
		}

		RHI_BufferView::RHI_BufferView(RHI_BufferView&& other)
		{
			m_buffer = other.m_buffer;
			m_offset = other.m_offset;
			m_size = other.m_size;

			other.m_buffer = nullptr;
			other.m_offset = 0;
			other.m_size = 0;
		}


		/// <summary>
		/// RHI_BufferView
		/// </summary>
		
		void RHI_DynamicBuffer::Create(RenderContext* context)
		{ 
			Release();
			m_context = context; 
			m_buffer = Renderer::CreateUniformBuffer(512);
		}

		void RHI_DynamicBuffer::Release()
		{
			if (m_buffer)
			{
				DeleteTracked(m_buffer);
			}
		}

		RHI_BufferView RHI_DynamicBuffer::Upload(void* data, int sizeInBytes)
		{
			if (m_size + sizeInBytes > m_capacity)
			{
				Resize(m_capacity * 2);
			}
			RHI_BufferView view = m_buffer->Upload(data, sizeInBytes, m_size);
			m_size += sizeInBytes;
			return view;
		}

		void RHI_DynamicBuffer::Resize(int newSizeInBytes)
		{
			if (m_buffer)
			{
				m_buffer->Resize(newSizeInBytes);
			}
		}

		void RHI_DynamicBuffer::Reset()
		{
			m_size = 0;
		}
	}
}
