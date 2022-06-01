#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/Enums.h"
#include <unordered_map>
#include "Graphics/RHI/RHI_Buffer.h"

namespace Insight
{
	namespace Graphics
	{
		class DescriptorBuffer;
		class RHI_Texture;

		struct Descriptor
		{
			Descriptor() { }
			Descriptor(int set, int binding, int stage, int size, DescriptorType type, DescriptorResourceType resourceType)
				: Set(set), Binding(binding), Stage(stage), Size(size), Type(type), ResourceType(resourceType)
			{ }
			Descriptor(int set, int binding, int stage, int size, DescriptorType type, DescriptorResourceType resourceType, RHI_BufferView view)
				: Set(set), Binding(binding), Stage(stage), Size(size), Type(type), ResourceType(resourceType), BufferView(view)
			{ }

			Descriptor(const Descriptor& other)
			{
				*this = other;
			}

			Descriptor(Descriptor&& other)
			{
				*this = std::move(other);
			}

			Descriptor& operator=(const Descriptor& other)
			{
				Set = other.Set;
				Binding = other.Binding;
				Stage = other.Stage;
				Size = other.Size;
				Type = other.Type;
				ResourceType = other.ResourceType;
				BufferView = other.BufferView;
				return *this;
			}

			Descriptor& operator=(Descriptor&& other)
			{

				Set = other.Set;
				Binding = other.Binding;
				Stage = other.Stage;
				Size = other.Size;
				Type = other.Type;
				ResourceType = other.ResourceType;
				BufferView = other.BufferView;

				return *this;
			}

			int Set = 0;
			int Binding = 0;
			int Stage = 0;
			int Size = 0;
			DescriptorType Type = DescriptorType::Unknown;
			DescriptorResourceType ResourceType = DescriptorResourceType::Unknown;
			
			RHI_BufferView BufferView;
			RHI_Texture* Texture = nullptr;

			u64 GetHash(bool includeResource) const
			{
				u64 hash = 0;

				HashCombine(hash, Set);
				HashCombine(hash, Binding);
				HashCombine(hash, Stage);
				HashCombine(hash, static_cast<u64>(Type));
				if (includeResource)
				{
					if (BufferView.IsValid())
					{
						HashCombine(hash, BufferView);
					}
					if (Texture)
					{
						HashCombine(hash, Texture);
					}
				}
				return hash;
			}
		};

		struct Uniform
		{
			Uniform() { }
			Uniform(int offset, int size, int set, int binding)
				: Offset(offset), Size(size), Set(set), Binding(binding)
			{ }

			int Offset = 0;
			int Size = 0;
			int Set = -1;
			int Binding = -1;
		};

		struct DescriptorBufferView
		{
			DescriptorBufferView() { }
			DescriptorBufferView(DescriptorBuffer* buffer, int offset, int sizeInBytes)
				: Buffer(buffer), Offset(offset), SizeInBytes(sizeInBytes)
			{ }

			DescriptorBuffer* Buffer = nullptr;
			int Offset = 0;
			int SizeInBytes = 0;
		};

		/// <summary>
		/// Handle CPU side uniform buffer to prepare for upload to GPU if required.
		/// </summary>
		class DescriptorBuffer
		{
		public:
			DescriptorBuffer();
			DescriptorBuffer(const DescriptorBuffer& other);
			DescriptorBuffer(DescriptorBuffer&& other);
			~DescriptorBuffer();

			DescriptorBuffer& operator=(const DescriptorBuffer& other);
			DescriptorBuffer& operator=(DescriptorBuffer&& other);

			DescriptorBufferView SetUniform(int set, int binding, void* data, int sizeInBytes);
			void Resize(int newCapacity);

			constexpr Byte* GetData() const { return m_buffer; }
			constexpr u32 GetSize() const { return m_size; }
			constexpr u32 GetCapacity() const { return m_capacity; }

			const std::unordered_map<int, std::unordered_map<int, std::vector<Uniform>>>& GetUniforms() const { return m_uniforms; }
			std::vector<Descriptor> GetDescriptors() const;

			void Reset();
			void Release();

		private:
			Byte* m_buffer = nullptr;
			int m_size = 0;
			int m_capacity = 0;
			std::unordered_map<int, std::unordered_map<int, std::vector<Uniform>>> m_uniforms;
		};
	}
}