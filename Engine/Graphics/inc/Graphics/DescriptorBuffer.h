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
		class RHI_Sampler;

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

			DescriptorType Type = DescriptorType::Unknown;
			u32 Set = 0;
			u32 Binding = 0;
			u32 Stage = 0;
			int Size = 0;
			DescriptorResourceType ResourceType = DescriptorResourceType::Unknown;
			
			RHI_BufferView BufferView;
			const RHI_Texture* Texture = nullptr;
			const RHI_Sampler* Sampler = nullptr;

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
					if (Sampler)
					{
						HashCombine(hash, Sampler);
					}
				}
				return hash;
			}
		};

		struct PushConstant
		{
			ShaderStageFlags ShaderStages;
			u32 Offset = 0;
			u32 Size = 0;
		};
	}
}