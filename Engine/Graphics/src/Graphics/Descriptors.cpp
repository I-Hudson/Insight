#include "Graphics/Descriptors.h"

namespace Insight
{
	namespace Graphics
	{
		DescriptorBinding::DescriptorBinding() NO_EXPECT
		{ }
		DescriptorBinding::DescriptorBinding(const u32 set, const u32 binding, const u32 stages, const u32 size, const u32 count, const DescriptorType type) NO_EXPECT
			: Set(set)
			, Binding(binding)
			, Stages(stages)
			, Size(size)
			, Count(count)
			, Type(type)
		{
			switch (Type)
			{
			case Insight::Graphics::DescriptorType::Sampler:
			{
				RHI_Sampler.resize(count);
				break;
			}
			case Insight::Graphics::DescriptorType::Sampled_Image:
			case Insight::Graphics::DescriptorType::Storage_Image:
			{
				RHI_Texture.resize(count);
				break;
			}
			case Insight::Graphics::DescriptorType::Unifom_Buffer:
			case Insight::Graphics::DescriptorType::Uniform_Buffer_Dynamic:
			case Insight::Graphics::DescriptorType::Storage_Buffer:
			case Insight::Graphics::DescriptorType::Storage_Buffer_Dyanmic:
			{
				RHI_Buffer_View.resize(count);
				break;
			}
			case Insight::Graphics::DescriptorType::Uniform_Texel_Buffer:
			case Insight::Graphics::DescriptorType::Storage_Texel_Buffer:
			case Insight::Graphics::DescriptorType::Input_Attachment:
			case Insight::Graphics::DescriptorType::Unknown:
			{
				FAIL_ASSERT();
				break;
			}
			default:
				break;
			}
		}
		/*DescriptorBinding::DescriptorBinding(const DescriptorBinding& other) NO_EXPECT
		{
			*this = other;
		}
		DescriptorBinding::DescriptorBinding(DescriptorBinding&& other) NO_EXPECT
		{
			*this = std::move(other);
		}*/

		/*DescriptorBinding& DescriptorBinding::operator=(const DescriptorBinding& other) NO_EXPECT
		{
			Set = other.Set;
			Binding = other.Binding;
			Stages = other.Stages;
			Size = other.Size;
			Type = other.Type;
			RHI_Buffer_View = other.RHI_Buffer_View;
			RHI_Texture = other.RHI_Texture;
			RHI_Sampler = other.RHI_Sampler;
			Hash_No_Resource = other.Hash_No_Resource;
			Hash_Resource = other.Hash_Resource;

			return *this;
		}
		DescriptorBinding& DescriptorBinding::operator=(DescriptorBinding&& other) NO_EXPECT
		{
			Set = other.Set;
			Binding = other.Binding;
			Stages = other.Stages;
			Size = other.Size;
			Type = other.Type;
			RHI_Buffer_View = other.RHI_Buffer_View;
			RHI_Texture = other.RHI_Texture;
			RHI_Sampler = other.RHI_Sampler;
			Hash_No_Resource = other.Hash_No_Resource;
			Hash_Resource = other.Hash_Resource;

			other.Set = -1;
			other.Binding = -1;
			other.Stages = -1;
			other.Size = 0;
			other.Type = DescriptorType::Unknown;
			other.RHI_Buffer_View = {};
			other.RHI_Texture = nullptr;
			other.RHI_Sampler = nullptr;
			other.Hash_No_Resource = 0;
			other.Hash_Resource = 0;

			return *this;
		}*/

		bool DescriptorBinding::operator==(const DescriptorBinding& other) const
		{
			return Set == other.Set
				&& Binding == other.Binding
				&& Stages == other.Stages
				&& Size == other.Size
				&& Type == other.Type
				&& RHI_Buffer_View == other.RHI_Buffer_View
				&& RHI_Texture == other.RHI_Texture
				&& RHI_Sampler == other.RHI_Sampler;
		}
		bool DescriptorBinding::operator!=(const DescriptorBinding& other) const
		{
			return !(*this == other);
		}

		void DescriptorBinding::SetHashs()
		{
			Hash_No_Resource = GetHash(false);
			Hash_Resource = GetHash(true);
		}

		u64 DescriptorBinding::GetHash(bool includeResource) const
		{
			u64 hash = 0;
			HashCombine(hash, Set);
			HashCombine(hash, Binding);
			HashCombine(hash, Stages);
			HashCombine(hash, static_cast<u64>(Type));
			if (includeResource)
			{
				for (size_t bufferIdx = 0; bufferIdx < RHI_Buffer_View.size(); ++bufferIdx)
				{
					const RHI_BufferView& buffer = RHI_Buffer_View[bufferIdx];
					if (buffer.IsValid())
					{
						if (Type == DescriptorType::Uniform_Buffer_Dynamic)
						{
							HashCombine(hash, buffer.GetBuffer());
							HashCombine(hash, buffer.GetSize());
						}
						else
						{
							HashCombine(hash, buffer);
						}
					}
				}

				for (size_t textureIdx = 0; textureIdx < RHI_Texture.size(); ++textureIdx)
				{
					const Graphics::RHI_Texture* texture = RHI_Texture[textureIdx];
					if (texture)
					{
						HashCombine(hash, texture);
					}
				}
				for (size_t samplerIdx = 0; samplerIdx < RHI_Sampler.size(); ++samplerIdx)
				{
					const Graphics::RHI_Sampler* sampler = RHI_Sampler[samplerIdx];
					if (sampler)
					{
						HashCombine(hash, sampler);
					}
				}
			}
			return hash;
		}

		//----------------------------------------------------------------
		// DescriptorSet
		//----------------------------------------------------------------
		DescriptorSet::DescriptorSet() NO_EXPECT
		{ }
		DescriptorSet::DescriptorSet(std::string name, int set, std::vector<DescriptorBinding> bindings) NO_EXPECT
			: Name(std::move(name))
			, Set(set)
			, Bindings(std::move(bindings))
		{ }
		/*DescriptorSet::DescriptorSet(const DescriptorSet& other) NO_EXPECT
		{
			*this = other;
		}
		DescriptorSet::DescriptorSet(DescriptorSet&& other) NO_EXPECT
		{ 
			*this = other;
		}*/


		/*DescriptorSet& DescriptorSet::operator=(const DescriptorSet& other) NO_EXPECT
		{
			Name = other.Name;
			Set = other.Set;
			Stages = other.Stages;
			Bindings = other.Bindings;
			Hash_No_Resource = other.Hash_No_Resource;
			Hash_Resource = other.Hash_Resource;
			return *this;
		}
		DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) NO_EXPECT
		{
			Name = std::move(other.Name);
			Set = std::move(other.Set);
			Stages = std::move(other.Stages);
			Bindings = std::move(other.Bindings);
			Hash_No_Resource = std::move(other.Hash_No_Resource);
			Hash_Resource = std::move(other.Hash_Resource);
			return *this;
		}*/

		bool DescriptorSet::operator==(const DescriptorSet& other) const
		{	
			if (Bindings.size() != other.Bindings.size())
			{
				return false;
			}

			const u64 binding_count = Bindings.size();
			for (size_t i = 0; i < binding_count; i++)
			{
				const DescriptorBinding& binding1 = Bindings.at(i);
				const DescriptorBinding& binding2 = other.Bindings.at(i);
				if (binding1 != binding2)
				{
					return false;
				}
			}
			return true;
		}
		bool DescriptorSet::operator!=(const DescriptorSet& other) const
		{
			return !(*this == other);
		}

		void DescriptorSet::SetHashs()
		{
			Hash_No_Resource = GetHash(false);
			Hash_Resource = GetHash(true);
		}

		u64 DescriptorSet::GetHash(bool includeResource) const
		{
			u64 hash = 0;
			for (size_t i = 0; i < Bindings.size(); ++i)
			{
				if (Bindings.at(i).Type != DescriptorType::Unknown)
				{
					HashCombine(hash, Bindings.at(i).GetHash(includeResource));
				}
			}
			return hash;
		}
	}
}