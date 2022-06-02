#include "Graphics/DescriptorBuffer.h"
#include "Core/Memory.h"
#include <assert.h>

#include <algorithm>

namespace Insight
{
	namespace Graphics
	{
		DescriptorBuffer::DescriptorBuffer()
		{
			Resize(2048);
		}

		DescriptorBuffer::DescriptorBuffer(const DescriptorBuffer& other)
		{
			*this = other;
		}

		DescriptorBuffer::DescriptorBuffer(DescriptorBuffer&& other)
		{
			*this = std::move(other);
		}

		DescriptorBuffer& DescriptorBuffer::operator=(const DescriptorBuffer& other)
		{
			//Resize(0); // Reset our current buffer to 0;
			Resize(other.m_capacity); // Resize our buffer to the same size as other's.
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

					for (const auto& uniform : binding.second)
					{
						m_uniforms[setIndex][bindingIndex].push_back(Uniform(uniform.Offset, uniform.Size, setIndex, bindingIndex));
					}
				}
			}
			return *this;
		}

		DescriptorBuffer& DescriptorBuffer::operator=(DescriptorBuffer&& other)
		{
			m_capacity = other.m_capacity;
			m_size = other.m_size;
			m_buffer = other.m_buffer;
			m_uniforms = std::move(other.m_uniforms);

			// Invalidate other data as we take control over it.
			// Not need but nice for explaining.
			other.m_capacity = 0;
			other.m_size = 0;
			other.m_buffer = nullptr;
			other.m_uniforms.clear();

			return *this;
		}

		DescriptorBuffer::~DescriptorBuffer()
		{
			Release();
		}

		DescriptorBufferView DescriptorBuffer::SetUniform(int set, int binding, void* data, int sizeInBytes)
		{
			if (m_size + sizeInBytes > m_capacity)
			{
				Resize(m_capacity * 2);
			}

			Byte* startPtr = m_buffer + m_size; // Get our start point in the buffer block of memory.
			memcpy_s(startPtr, sizeInBytes, data, sizeInBytes); // Copy data info the block of memory.

			const int offset = m_size;
			m_uniforms[set][binding].push_back(Uniform(offset, sizeInBytes, set, binding));

			m_size += AlignUp(sizeInBytes, 256); // Increase size member. Align with 256.
			return DescriptorBufferView(this, offset, AlignUp(sizeInBytes, 256));
		}

		void DescriptorBuffer::Resize(int newCapacity)
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

			if (m_buffer)
			{
				// We need to reallocte.
				void* newBlock = realloc(m_buffer, newCapacity);
				assert(newBlock != nullptr && "[DescriptorBuffer::Resize] Unable to realloc buffer.");

				memcpy_s(newBlock, newCapacity, m_buffer, m_capacity);
				m_capacity = newCapacity;

				UntrackPtr(m_buffer);
				m_buffer = (Byte*)newBlock;
				TrackPtr(m_buffer);
				return;
			}

			m_capacity = newCapacity;
			m_buffer = (Byte*)malloc(m_capacity);
			TrackPtr(m_buffer);
		}

		std::vector<Descriptor> DescriptorBuffer::GetDescriptorsSignature() const
		{
			std::vector<Descriptor> descriptors;
			for (const auto& sets : m_uniforms)
			{
				const int setIndex = sets.first;
				for (const auto& bindings : sets.second)
				{
					const int bindingIndex = bindings.first;
					const int descriptorStartIndex = (int)descriptors.size();

					descriptors.push_back(Descriptor(setIndex, bindingIndex, 0, 0, DescriptorType::Unifom_Buffer, DescriptorResourceType::CBV));

					std::sort(descriptors.begin() + descriptorStartIndex, descriptors.end(), [](const Descriptor& d1, const Descriptor& d2)
						{
							return d1.Binding < d2.Binding;
						});
				}
			}
			return descriptors;
		}

		void DescriptorBuffer::Reset()
		{
			m_size = 0;
			m_uniforms.clear();
			memset(m_buffer, 0, m_capacity);
		}

		void DescriptorBuffer::Release()
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