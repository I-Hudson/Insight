#include "Graphics/RHI/RHI_Buffer.h"
#include "Core/Memory.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/RenderContext.h"

#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_Buffer* RHI_Buffer::New()
		{
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_Buffer_Vulkan); }
			else if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_Buffer_DX12); }
			return nullptr;
		}

		RHI_BufferView RHI_Buffer::GetView(int offset, int size)
		{
			return RHI_BufferView(this, offset, size);
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

		RHI_BufferView& RHI_BufferView::operator=(const RHI_BufferView& other)
		{
			m_buffer = other.m_buffer;
			m_offset = other.m_offset;
			m_size = other.m_size;
			return *this;
		}

		RHI_BufferView& RHI_BufferView::operator=(RHI_BufferView&& other)
		{
			m_buffer = other.m_buffer;
			m_offset = other.m_offset;
			m_size = other.m_size;

			other.m_buffer = nullptr;
			other.m_offset = 0;
			other.m_size = 0;

			return *this;
		}


		/// <summary>
		/// RHI_BufferView
		/// </summary>
		
		void RHI_DynamicBuffer::Create(RenderContext* context)
		{ 
			Release();
			m_context = context; 
			m_buffer = Renderer::CreateUniformBuffer(512);
			m_buffer->SetName(L"Constent_Dynamic_Buffer");
			m_size = 0;
			m_capacity = 512;
		}

		void RHI_DynamicBuffer::Release()
		{
			if (m_buffer)
			{
				Renderer::FreeUniformBuffer(m_buffer);
				m_buffer = nullptr;
			}
		}

		RHI_BufferView RHI_DynamicBuffer::Upload(void* data, int sizeInBytes)
		{
			sizeInBytes = std::max(0, sizeInBytes);
			if (sizeInBytes == 0 || data == nullptr)
			{
				return RHI_BufferView(nullptr, 0, 0);
			}

			if (m_size + sizeInBytes > m_capacity)
			{
				Resize(m_capacity * 2);
			}
			RHI_BufferView view = m_buffer->Upload(data, sizeInBytes, m_size);
			m_size += sizeInBytes;
			return view;
		}

		RHI_BufferView RHI_DynamicBuffer::GetView(int offset, int size) const
		{
			if (m_buffer)
			{
				return m_buffer->GetView(offset, size);
			}
			return RHI_BufferView();
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
