#include "Graphics/UniformBuffer.h"
#include "Core/Memory.h"
#include <assert.h>

namespace Insight
{
	namespace Graphics
	{
		UniformBuffer::UniformBuffer()
		{
			Resize(12);
		}

		UniformBuffer::UniformBuffer(const UniformBuffer& other)
		{
			*this = other;
		}

		UniformBuffer::UniformBuffer(UniformBuffer&& other)
		{
			*this = std::move(other);
		}

		UniformBuffer& UniformBuffer::operator=(const UniformBuffer& other)
		{
			Resize(0); // Reset our current buffer to 0;
			Resize(other.m_capacity); // Resize our buffer to the same size as other;s.
			m_size = other.m_size;

			memcpy_s(m_buffer, m_capacity, other.m_buffer, other.m_capacity); // Call all contents from other into this buffer.

			// Go through all other's set and binding and copy them to here.
			// This is needed as we keep local Ptr references to where in m_buffer 
			// out data is. This needs to be replicated here.

			const Byte* otherStartPtr = other.m_buffer;
			for (const auto& set : other.m_uniforms)
			{
				const int setIndex = set.first;
				for (const auto& binding : set.second)
				{
					const int bindingIndex = binding.first;
					const Uniform otherUniform = binding.second;
					const int bufferOffset = otherUniform.Ptr - otherStartPtr;

					m_uniforms[setIndex][bindingIndex] = Uniform(m_buffer + bufferOffset, otherUniform.Size, setIndex, bindingIndex);
				}
			}
			return *this;
		}

		UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other)
		{
			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_buffer = other.m_buffer;
			m_uniforms = other.m_uniforms;

			// Invalidate other data as we take control over it.
			// Not need but nice for explaining.
			other.m_capacity = 0;
			other.m_size = 0;
			other.m_buffer = nullptr;
			other.m_uniforms.clear();

			return *this;
		}

		UniformBuffer::~UniformBuffer()
		{
			Release();
		}

		void UniformBuffer::SetUniform(int set, int binding, void* data, int sizeInBytes)
		{
			if (m_size + sizeInBytes > m_capacity)
			{
				Resize(m_capacity * 2);
			}

			Byte* startPtr = m_buffer + m_size; // Get our start point in the buffer block of memory.
			memcpy_s(startPtr, sizeInBytes, data, sizeInBytes); // Copy data info the block of memory.
			m_size += sizeInBytes; // Increase size member.
			m_uniforms[set][binding] = Uniform(startPtr, sizeInBytes, set, binding);
		}

		void UniformBuffer::Resize(int newCapacity)
		{
			// Clamp newCapacity to 0 and above.
			newCapacity = std::max(0, newCapacity);

			if (newCapacity == 0)
			{
				m_capacity = newCapacity;
				m_size = 0;
				if (m_buffer)
				{
					Release();
				}
				return;
			}

			if (m_capacity == newCapacity)
			{
				return;
			}
			m_capacity = newCapacity;

			if (m_buffer)
			{
				// We need to reallocte.
				void* newBlock = realloc(m_buffer, newCapacity);
				assert(newBlock != nullptr && "[UniformBuffer::Resize] Unable to realloc buffer.");

				UntrackPtr(m_buffer);
				m_buffer = (Byte*)newBlock;
				TrackPtr(m_buffer);
				return;
			}

			m_buffer = (Byte*)malloc(m_capacity);
			TrackPtr(m_buffer);
		}

		void UniformBuffer::Reset()
		{
			m_size = 0;
			m_uniforms.clear();
		}

		void UniformBuffer::Release()
		{
			if (m_buffer)
			{
				UntrackPtr(m_buffer);
				free(m_buffer);
				m_buffer = 0;
			}
		}


	}
}