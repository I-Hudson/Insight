#pragma once

#include "Engine/Core/InsightAlias.h"

namespace Insight::Collections
{
	/// <summary>
	/// Static buffer object. Allocate a block on memory. Can not be resized.
	/// </summary>
	class Buffer
	{
	public:
		Buffer() = delete;
		Buffer(u64 const& size);
		Buffer(Buffer const& other);
		Buffer(Buffer&& other);
		~Buffer();

		/// <summary>
		/// Clear the buffer of all data. 
		/// </summary>
		void Clear();

		/// <summary>
		/// Explicitly clear and delete this buffer from memory.
		/// </summary>
		void Destroy();

		Byte const* GetPtr() const { return m_buffer; }
		u64 const& GetSize() const { return m_size; }
		u64 const& GetTop() const { return m_top; }

	private:
		Byte* m_buffer;
		u64 m_size;
		u64 m_top;
	};
}