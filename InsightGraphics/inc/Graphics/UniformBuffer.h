#pragma once

#include "Core/TypeAlias.h"
#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		struct Uniform
		{
			Uniform() { }
			Uniform(Byte* ptr, int size, int set, int binding)
				: Ptr(ptr), Size(size), Set(set), Binding(binding)
			{ }

			Byte* Ptr = nullptr;
			int Size = 0;
			int Set = -1;
			int Binding = -1;
		};

		/// <summary>
		/// Handle CPU side uniform buffer to prepare for upload to GPU if required.
		/// </summary>
		class UniformBuffer
		{
		public:
			UniformBuffer();
			UniformBuffer(const UniformBuffer& other);
			UniformBuffer(UniformBuffer&& other);
			~UniformBuffer();

			UniformBuffer& operator=(const UniformBuffer& other);
			UniformBuffer& operator=(UniformBuffer&& other);

			void SetUniform(int set, int binding, void* data, int sizeInBytes);
			void Resize(int newCapacity);

			void Reset();
			void Release();

		private:
			Byte* m_buffer = nullptr;
			int m_size = 0;
			int m_capacity = 0;
			std::unordered_map<int, std::unordered_map<int, Uniform>> m_uniforms;
		};
	}
}