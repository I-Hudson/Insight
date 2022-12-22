#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/Defines.h"
#include "Graphics/Enums.h"
#include "Graphics/RHI/RHI_Buffer.h"

#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class DescriptorBuffer;
		class RHI_Texture;
		class RHI_Sampler;

		struct IS_GRAPHICS DescriptorBinding
		{
			DescriptorBinding() NO_EXPECT;
			DescriptorBinding(int set, int binding, int stages, u32 size, DescriptorType type) NO_EXPECT;
			DescriptorBinding(const DescriptorBinding& other) NO_EXPECT;
			DescriptorBinding(DescriptorBinding&& other) NO_EXPECT;

			DescriptorBinding& operator=(const DescriptorBinding& other) NO_EXPECT;
			DescriptorBinding& operator=(DescriptorBinding&& other) NO_EXPECT;

			bool operator==(const DescriptorBinding& other) const;
			bool operator!=(const DescriptorBinding& other) const;

			void SetHashs();
			u64 GetHash(bool includeResource) const;

			u32 Set = 0;
			u32 Binding = 0;
			u32 Stages = 0;
			u32 Size = 0;
			DescriptorType Type = DescriptorType::Unknown;

			u64 Hash_No_Resource = 0;
			u64 Hash_Resource = 0;

			RHI_BufferView RHI_Buffer_View = { };
			const RHI_Texture* RHI_Texture = nullptr;
			const RHI_Sampler* RHI_Sampler = nullptr;
		};

		struct IS_GRAPHICS DescriptorSet
		{
			DescriptorSet() NO_EXPECT;
			DescriptorSet(std::string name, int set, std::vector<DescriptorBinding> bindings) NO_EXPECT;
			DescriptorSet(const DescriptorSet& other) NO_EXPECT;
			DescriptorSet(DescriptorSet&& other) NO_EXPECT;

			DescriptorSet& operator=(const DescriptorSet& other) NO_EXPECT;
			DescriptorSet& operator=(DescriptorSet&& other) NO_EXPECT;

			bool operator==(const DescriptorSet& other) const;
			bool operator!=(const DescriptorSet& other) const;

			void SetHashs();
			u64 GetHash(bool includeResource) const;

			u64 Hash_No_Resource = 0;
			u64 Hash_Resource = 0;

			std::string Name;
			u32 Set = 0;
			u32 Stages = 0;
			u32 Size = 0;
			std::vector<DescriptorBinding> Bindings;
		};

		struct PushConstant
		{
			ShaderStageFlags ShaderStages;
			u32 Offset = 0;
			u32 Size = 0;
		};
	}
}