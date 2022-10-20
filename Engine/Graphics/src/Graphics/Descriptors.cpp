#include "Graphics/Descriptors.h"

namespace Insight
{
	namespace Graphics
	{
		DescriptorBinding::DescriptorBinding() NO_EXPECT
		{ }
		DescriptorBinding::DescriptorBinding(int set, int binding, int stages, u32 size, DescriptorType type) NO_EXPECT
			: Set(set)
			, Binding(binding)
			, Stages(stages)
			, Size(size)
			, Type(type)
		{ }
		DescriptorBinding::DescriptorBinding(const DescriptorBinding& other) NO_EXPECT
		{
			*this = other;
		}
		DescriptorBinding::DescriptorBinding(DescriptorBinding&& other) NO_EXPECT
		{
			*this = std::move(other);
		}

		DescriptorBinding& DescriptorBinding::operator=(const DescriptorBinding& other) NO_EXPECT
		{
			Set = other.Set;
			Binding = other.Binding;
			Stages = other.Stages;
			Size = other.Size;
			Type = other.Type;
			RHI_Buffer_View = other.RHI_Buffer_View;
			RHI_Texture = other.RHI_Texture;
			RHI_Sampler = other.RHI_Sampler;

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

			other.Set = -1;
			other.Binding = -1;
			other.Stages = -1;
			other.Size = 0;
			other.Type = DescriptorType::Unknown;
			other.RHI_Buffer_View = {};
			other.RHI_Texture = nullptr;
			other.RHI_Sampler = nullptr;

			return *this;
		}

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
				if (RHI_Buffer_View.IsValid())
				{
					HashCombine(hash, RHI_Buffer_View);
				}
				if (RHI_Texture)
				{
					HashCombine(hash, RHI_Texture);
				}
				if (RHI_Sampler)
				{
					HashCombine(hash, RHI_Sampler);
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
		DescriptorSet::DescriptorSet(const DescriptorSet& other) NO_EXPECT
			: Name(other.Name)
			, Set(other.Set)
			, Bindings(other.Bindings)
		{ }
		DescriptorSet::DescriptorSet(DescriptorSet&& other) NO_EXPECT
			: Name(std::move(other.Name))
			, Set(std::move(other.Set))
			, Bindings(std::move(other.Bindings))
		{ }


		DescriptorSet& DescriptorSet::operator=(const DescriptorSet& other) NO_EXPECT
		{
			Name = other.Name;
			Set = other.Set;
			Bindings = other.Bindings;
			return *this;
		}
		DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) NO_EXPECT
		{
			Name = std::move(other.Name);
			Set = std::move(other.Set);
			Bindings = std::move(other.Bindings);
			return *this;
		}

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
				HashCombine(hash, Bindings.at(i).GetHash(includeResource));
			}
			return hash;
		}
	}
}